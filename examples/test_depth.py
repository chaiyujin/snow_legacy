import numpy as np
from anime_viewer import anime_viewer11
from depth_video import depth11
from bilateral_filter import BilateralFilter1D

PATH_DIR = "/media/chaiyujin/FE6C78966C784B81/Linux/Dataset/MyDepth/netural/"

depth11.set_fps(40.0)
depth11.set_sample_rate(20480)

def view_video(video_path):
    result = depth11.collect_video(video_path)
    if result is None:
        print("error")
    else:
        filter = BilateralFilter1D(
            factor=-0.5, distance_sigma=1,
            range_sigma=1, radius=1)
        result["expr_frames"] = filter.filter(result["expr_frames"]) * 1.3 
        result["expr_frames"][:, :20] = 0
        result["iden"][1:] = 0
        anime_viewer11.initialize_bilinear("../assets/fw/")
        anime_viewer11.new_app("bilinear")
        anime_viewer11.set_iden("win", result["iden"])
        anime_viewer11.set_expr_list("win", result["expr_frames"])
        anime_viewer11.add_audio_f32("audio", result["audio"], result["sample_rate"])
        anime_viewer11.run(result["fps"])
        anime_viewer11.terminate()


def view_test():
    iden = np.zeros((75))
    iden[0] = 1.0
    wav = np.load("wav_0.npy")
    pred_list = np.swapaxes(np.load("pred_0.npy"), 0, 1)
    true_list = np.swapaxes(np.load("true_0.npy"), 0, 1)
    anime_viewer11.initialize_bilinear("../assets/fw/")
    anime_viewer11.new_app("bilinear")
    anime_viewer11.set_iden("pred", iden)
    anime_viewer11.set_expr_list("pred", pred_list)
    anime_viewer11.set_iden("true", iden)
    anime_viewer11.set_expr_list("true", true_list)
    anime_viewer11.add_audio_f32("audio", wav, 20480)
    anime_viewer11.run(40.0)
    anime_viewer11.terminate()

view_test()


# for i in range(150):
#     video_path = PATH_DIR + "0-{}-1.mkv".format(i)
#     print(video_path)
#     view_video(video_path)
