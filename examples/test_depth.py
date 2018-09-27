from anime_viewer import anime_viewer11
from depth_video import depth11

depth11.set_fps(40.0)
depth11.set_sample_rate(20480)
result = depth11.collect_video("../assets/test_depth/0-0-1.mkv")
if result is None:
    print("error")
else:
    anime_viewer11.initialize_bilinear("../assets/fw/")
    anime_viewer11.new_app("bilinear")
    anime_viewer11.set_iden("win", result["iden"])
    anime_viewer11.set_expr_list("win", result["expr_frames"])
    anime_viewer11.add_audio_f32("audio", result["audio"], result["sample_rate"])
    anime_viewer11.run(result["fps"])
    anime_viewer11.terminate()
