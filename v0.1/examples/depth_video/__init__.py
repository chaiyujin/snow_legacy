try:
    from .build import depth11
except:
    from build import depth11


if __name__ == "__main__":
    import numpy as np
    import librosa

    depth11.set_sample_rate(16000)
    result = depth11.collect_video("../../assets/test_depth/0-0-1.mkv")
    librosa.output.write_wav("../../assets/test_depth/audio.wav", result["audio"], result["sample_rate"])
    for k in result:
        v = result[k]
        if np.isscalar(v):
            print(k.ljust(15), v)
        else:
            print(k.ljust(15), v.shape)
