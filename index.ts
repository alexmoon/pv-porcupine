import bindings from "bindings";
import { Writable, WritableOptions } from "stream";

export interface KeywordOptions {
    /**
     * Absolute path to keyword file.
     */
    filePath: string;

    /**
     * @param sensitivity Sensitivity for detecting keyword. It should be a floating-point number within
     * [0, 1]. A higher sensitivity value results in fewer misses at the cost of increasing the false alarm rate.
     */
    sensitivity: number;
}

type KeywordSpec = string | KeywordOptions;

declare class Porcupine {
    /**
     * @param moduleFilePath Absolute path to file containing model parameters.
     * @param keywords Either a single keyword or an array of keywords. If a string is provided for a keyword,
     * it must be the absolute path to the keyword file and the sensitivity of will be set to 0.5 for that keyword.
     */
    constructor(moduleFilePath: string, keywords: KeywordSpec | KeywordSpec[]);

    /**
     * Destructor.
     */
    public destroy(): void;

    /**
     * Monitors incoming audio stream for a given keyword.
     *
     * @param pcm A frame of audio samples. The number of samples per frame can be attained by calling
     * 'porcupine.frameLength()'. The incoming audio needs to have a sample rate equal to 'porcupine.sampleRate()' and be 16-bit
     * linearly-encoded. Furthermore, porcupine operates on single channel audio.
     * @return Flag indicating if the keyword has been observed ending at the current frame (if a single keyword was passed to
     * the constructor) or index of observed keyword at the end of current frame (if an array of keywords was passed to the
     * constructor). Indexing is 0-based and based on the ordering of 'keywords' passed to the constructor. If no keyword is
     * detected -1 is returned.
     */
    public process(pcm: Buffer): boolean | number;
}

type PorcupineModule = {
    /**
     * Forward declaration for keyword spotting object (a.k.a. porcupine).
     * The object detects utterances of a given keyword(s) within incoming stream of audio in real-time.
     * It processes incoming audio in consecutive frames (chucks) and for each frame emits result of detecting the keyword(s)
     * ending at that frame. The number of samples per frame can be attained by calling 'pv_porcupine_frame_length()'.
     * The incoming audio needs to have a sample rate equal to 'pv_sample_rate()' and be 16-bit linearly-encoded. Furthermore,
     * porcupine operates on single channel audio.
     */
    Porcupine: typeof Porcupine,

    /**
     * Getter for version string.
     *
     * @return Version.
     */
    version(): string,

    /**
     * Audio sample rate accepted by Picovoice.
     */
    sampleRate(): number,

    /**
     * Getter for length (number of audio samples) per frame.
     *
     * @return frame length.
     */
    frameLength(): number,
};

const porcupine = bindings("pv_porcupine.node") as PorcupineModule;
export default porcupine;

interface PorcupineStreamOptions extends WritableOptions {
    /**
     * Absolute path to file containing model parameters.
     */
    moduleFilePath: string;

    /**
     * Either a single keyword or an array of keywords. If a string is provided for a keyword, it must be the absolute path to
     * the keyword file and the sensitivity of will be set to 0.5 for that keyword.
     */
    keywords: KeywordSpec | KeywordSpec[];
}

/**
 * PorcupineStream is a writeable stream that emits "keyword" events when a keyword is detected.
 */
export class PorcupineStream extends Writable {
    private porcupine: Porcupine;
    private buffer: Buffer;
    private bufferLength = 0;
    private frameSize: number;

    constructor(options: PorcupineStreamOptions) {
        super(options);
        this.porcupine = new porcupine.Porcupine(options.moduleFilePath, options.keywords);
        this.frameSize = porcupine.frameLength() * 2;
        this.buffer = Buffer.allocUnsafe(this.frameSize);
    }

    public _write(chunk: any, encoding: string, callback: (err?: Error) => void): void {
        if (typeof chunk === "string") {
            chunk = Buffer.from(chunk, encoding);
        }

        if (!(chunk instanceof Buffer)) {
            callback(new Error("PorcupineStream does not support object mode."));
            return;
        }

        const frameSize = this.frameSize;
        let offset = 0;
        if (this.bufferLength > 0) {
            const bytesToCopy = Math.min(frameSize - this.bufferLength, chunk.length);
            chunk.copy(this.buffer, this.bufferLength, 0, bytesToCopy);
            this.bufferLength += bytesToCopy;
            offset = bytesToCopy;

            if (this.bufferLength === frameSize) {
                const result = this.porcupine.process(this.buffer);
                this.bufferLength = 0;
                if (result !== false && result !== -1) {
                    this.emit("keyword", chunk.slice(offset), result === true ? 0 : result);
                    callback();
                    return;
                }
            }
        }

        while (offset + frameSize <= chunk.length) {
            const result = this.porcupine.process(chunk.slice(offset, offset + frameSize));
            offset += frameSize;
            if (result !== false && result !== -1) {
                this.emit("keyword", chunk.slice(offset), result === true ? 0 : result);
                callback();
                return;
            }
        }

        if (offset < chunk.length) {
            chunk.copy(this.buffer, 0, offset);
            this.bufferLength = chunk.length - offset;
        }

        callback();
    }

    public _destroy(err: Error | undefined, callback: (err?: Error) => void): void {
        this.porcupine.destroy();
        callback(err);
    }
}

export interface PorcupineStream extends Writable {
    /**
     * Event emitter method signatures for the "keyword" event.
     */
    addListener(event: string, listener: (...args: any[]) => void): this;
    addListener(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;

    emit(event: string | symbol, ...args: any[]): boolean;
    emit(event: "keyword", chunk: Buffer, keyword: number): boolean;

    on(event: string, listener: (...args: any[]) => void): this;
    on(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;

    once(event: string, listener: (...args: any[]) => void): this;
    once(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;

    prependListener(event: string, listener: (...args: any[]) => void): this;
    prependListener(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;

    prependOnceListener(event: string, listener: (...args: any[]) => void): this;
    prependOnceListener(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;

    removeListener(event: string, listener: (...args: any[]) => void): this;
    removeListener(event: "keyword", listener: (chunk: Buffer, keyword: number) => void): this;
}
