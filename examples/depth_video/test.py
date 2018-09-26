from build import depth11
import librosa

depth11.set_sample_rate(16000)
result = depth11.collect_video("../../assets/test_depth/0-0-1.mkv")
librosa.output.write_wav("../../assets/test_depth/audio.wav", result["audio"], result["sample_rate"])
