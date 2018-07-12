#include <nan.h>
#include <string>
#include <pv_porcupine.h>

namespace node_porcupine
{

void HandlePicovoiceStatus(pv_status_t status)
{
    switch (status)
    {
    case PV_STATUS_SUCCESS:
        break;
    case PV_STATUS_OUT_OF_MEMORY:
        Nan::ThrowError("Porcupine out of memory");
        break;
    case PV_STATUS_IO_ERROR:
        Nan::ThrowError("Porcupine IO error");
        break;
    case PV_STATUS_INVALID_ARGUMENT:
        Nan::ThrowError("Porcupine invalid argument");
        break;
    default:
        Nan::ThrowError("Porcupine error");
    }
}

class Porcupine : public Nan::ObjectWrap
{
  public:
    static size_t FRAME_SIZE;
    static NAN_MODULE_INIT(Init);

    Porcupine(pv_porcupine_object_t *object, bool multipleKeywords) : m_object(object), m_multipleKeywords(multipleKeywords)
    {
    }

    ~Porcupine() throw()
    {
        destroy();
    }

    void destroy()
    {
        if (m_object)
        {
            pv_porcupine_delete(m_object);
            m_object = NULL;
        }
    }

    v8::Local<v8::Value> process(const int16_t *pcm)
    {
        v8::Local<v8::Value> result;
        pv_status_t status;
        if (m_multipleKeywords)
        {
            int keyword_index;
            status = pv_porcupine_multiple_keywords_process(m_object, pcm, &keyword_index);
            if (status == PV_STATUS_SUCCESS)
            {
                result = Nan::New<v8::Int32>(keyword_index);
            }
        }
        else
        {
            bool r;
            status = pv_porcupine_process(m_object, pcm, &r);
            if (status == PV_STATUS_SUCCESS)
            {
                result = Nan::New(r);
            }
        }

        HandlePicovoiceStatus(status);
        return result;
    }

  private:
    static NAN_METHOD(New);
    static NAN_METHOD(Destroy);
    static NAN_METHOD(Process);

    pv_porcupine_object_t *m_object;
    bool m_multipleKeywords;
};

size_t Porcupine::FRAME_SIZE;

static inline Nan::Persistent<v8::Function> &constructor()
{
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
}

NAN_MODULE_INIT(Porcupine::Init)
{
    Porcupine::FRAME_SIZE = sizeof(int16_t) * pv_porcupine_frame_length();

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Porcupine").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "destroy", Destroy);
    SetPrototypeMethod(tpl, "process", Process);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("Porcupine").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

static bool parseKeyword(v8::Local<v8::Value> keyword, std::string &keyword_file_path, float &sensitivity)
{
    if (keyword->IsString())
    {
        keyword_file_path = *Nan::Utf8String(Nan::To<v8::String>(keyword).ToLocalChecked());
        sensitivity = 0.5;
        return true;
    }
    else if (keyword->IsObject())
    {
        v8::Local<v8::Object> keywordObject = Nan::To<v8::Object>(keyword).ToLocalChecked();
        v8::Local<v8::String> filePathKeyStr = Nan::New<v8::String>("filePath").ToLocalChecked();
        v8::Local<v8::String> sensitivityKeyStr = Nan::New<v8::String>("sensitivity").ToLocalChecked();
        if (!Nan::Has(keywordObject, filePathKeyStr).FromJust())
        {
            return false;
        }

        v8::MaybeLocal<v8::String> filePathVal = Nan::To<v8::String>(Nan::Get(keywordObject, filePathKeyStr).ToLocalChecked());
        v8::Maybe<double> sensitivityVal = Nan::To<double>(Nan::Get(keywordObject, sensitivityKeyStr).ToLocalChecked());
        if (filePathVal.IsEmpty() || sensitivityVal.IsNothing())
        {
            return false;
        }

        keyword_file_path = *Nan::Utf8String(filePathVal.ToLocalChecked());
        sensitivity = sensitivityVal.FromMaybe(0.5);
        return true;
    }
    else
    {
        return false;
    }
}

// type Keyword = { file_path: string, sensitivity: number }
// new Porcupine(model_file_path: string, keywords: Keyword | Keyword[])
NAN_METHOD(Porcupine::New)
{
    if (info.IsConstructCall())
    {
        if ((info.Length() < 2) || !info[0]->IsString())
        {
            return Nan::ThrowError("Porcupine constructor requires a model file path and one or more keyword specifications as arguments");
        }

        const char *model_file_path = *Nan::Utf8String(info[0]);
        pv_porcupine_object_t *object;
        pv_status_t status;
        bool multiple_keywords = info[1]->IsArray();
        if (multiple_keywords)
        {
            v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(info[1]);
            uint32_t length = array->Length();
            std::unique_ptr<std::string, std::default_delete<std::string[]>> keyword_file_path_strings(new std::string[length]);
            std::unique_ptr<const char *, std::default_delete<const char *[]>> keyword_file_paths(new const char *[length]);
            std::unique_ptr<float, std::default_delete<float[]>> sensitivities(new float[length]);
            for (uint32_t i = 0; i < length; i++)
            {
                if (!parseKeyword(Nan::Get(array, i).ToLocalChecked(), keyword_file_path_strings.get()[i], sensitivities.get()[i]))
                {
                    return Nan::ThrowError("Invalid keyword specification for Porcupine constructor");
                }
                else
                {
                    keyword_file_paths.get()[i] = keyword_file_path_strings.get()[i].c_str();
                }
            }

            status = pv_porcupine_multiple_keywords_init(model_file_path, length, keyword_file_paths.get(), sensitivities.get(), &object);
        }
        else
        {
            std::string keyword_file_path;
            float sensitivity;
            if (!parseKeyword(info[1], keyword_file_path, sensitivity))
            {
                return Nan::ThrowError("Invalid keyword specification for Porcupine constructor");
            }

            status = pv_porcupine_init(model_file_path, keyword_file_path.c_str(), sensitivity, &object);
        }

        HandlePicovoiceStatus(status);
        Porcupine *obj = new Porcupine(object, multiple_keywords);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        const int argc = 1;
        v8::Local<v8::Value> argv[] = {info[0]};
        v8::Local<v8::Function> cons = Nan::New(constructor());
        info.GetReturnValue().Set(cons->NewInstance(Nan::GetCurrentContext(), argc, argv).ToLocalChecked());
    }
}

NAN_METHOD(Porcupine::Destroy)
{
    Porcupine *obj = Nan::ObjectWrap::Unwrap<Porcupine>(info.Holder());
    obj->destroy();
    info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Porcupine::Process)
{
    if (info.Length() < 1 || !info[0]->IsArrayBufferView())
    {
        return Nan::ThrowError("Porcupine::process requires a Buffer argument");
    }
    if (node::Buffer::Length(info[0]) != FRAME_SIZE) {
        return Nan::ThrowError("Porcupine::process Buffer size must match frameLength()");
    }
    Porcupine *obj = Nan::ObjectWrap::Unwrap<Porcupine>(info.Holder());
    v8::Local<v8::Value> result = obj->process(reinterpret_cast<const int16_t*>(node::Buffer::Data(info[0])));
    info.GetReturnValue().Set(result);
}

NAN_METHOD(sampleRate)
{
    v8::Local<v8::Int32> result = Nan::New<v8::Int32>(pv_sample_rate());
    info.GetReturnValue().Set(result);
}

NAN_METHOD(version)
{
    v8::Local<v8::String> result = Nan::New<v8::String>(pv_porcupine_version()).ToLocalChecked();
    info.GetReturnValue().Set(result);
}

NAN_METHOD(frameLength)
{
    v8::Local<v8::Int32> result = Nan::New<v8::Int32>(pv_porcupine_frame_length());
    info.GetReturnValue().Set(result);
}

NAN_MODULE_INIT(ModuleInit)
{
    Porcupine::Init(target);
    NAN_EXPORT(target, sampleRate);
    NAN_EXPORT(target, version);
    NAN_EXPORT(target, frameLength);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, ModuleInit)

} // namespace node_porcupine
