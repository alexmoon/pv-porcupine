{
  "targets": [
    {
      "target_name": "pv_porcupine",
      "sources": [ "src/porcupine.cc" ],
      "include_dirs": [
          "<!(node -e \"require('nan')\")",
          "Porcupine/include"
      ],
      "conditions" : [
        [
          'OS=="mac"', {
            'xcode_settings': {
              'GCC_ENABLE_CPP_RTTI': 'YES',
              'MACOSX_DEPLOYMENT_TARGET': '10.13',
              'OTHER_CPLUSPLUSFLAGS': [
                '-std=c++11',
                '-stdlib=libc++',
                '-fexceptions'
              ],
              'OTHER_LDFLAGS': [
                "-Wl,-rpath,@loader_path"
              ]
            },
            "link_settings": {
              "libraries": [
                "<@(module_root_dir)/Porcupine/lib/mac/x86_64/libpv_porcupine.a"
              ]
            }
          },
        ]
      ]
    }
  ]
}
