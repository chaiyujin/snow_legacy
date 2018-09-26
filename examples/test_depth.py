# from anime_viewer import anime_viewer11
# from depth_video import depth11

# depth11.set_sample_rate(22050)
# result = depth11.collect_video("../assets/test_depth/0-0-1.mkv")
# if result is None:
#     print("error")
# else:
#     print(result["iden"])
#     anime_viewer11.initialize_bilinear("../assets/fw/")
#     anime_viewer11.new_app("bilinear")
#     anime_viewer11.set_iden("win", result["iden"])
#     anime_viewer11.set_expr_list("win", result["expr_frames"])
#     anime_viewer11.run(30)
#     anime_viewer11.terminate()


def test_anime_viewer():
    import numpy as np
    from anime_viewer import anime_viewer11
    iden = np.zeros((50), dtype=np.float)
    expr_frames = np.zeros((100, 47), dtype=np.float)
    iden[0] = 1.0
    
    anime_viewer11.initialize_bilinear("../assets/fw/")
    # for _ in range(10):
    #     anime_viewer11.new_app("bilinear")
    #     anime_viewer11.set_iden("win", iden)
    #     anime_viewer11.set_expr_list("win", expr_frames)
    #     anime_viewer11.run(30)
    #     anime_viewer11.terminate()

test_anime_viewer()
