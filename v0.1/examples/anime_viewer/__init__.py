try:
    from .build import anime_viewer11
except:
    from build import anime_viewer11


def test_anime_viewer():
    import numpy as np
    iden = np.zeros((50), dtype=np.float)
    expr_frames = np.zeros((100, 47), dtype=np.float)
    iden[0] = 1.0
    
    anime_viewer11.initialize_bilinear("../../assets/fw/")
    # for _ in range(1):
    #     anime_viewer11.new_app("bilinear")
    #     anime_viewer11.set_iden("win", iden)
    #     anime_viewer11.set_expr_list("win", expr_frames)
    #     anime_viewer11.run(30)
    #     anime_viewer11.terminate()

def test_obj():
    anime_viewer11.new_app("obj")
    anime_viewer11.set_obj("win", "../../assets/F1/F1.obj")
    anime_viewer11.run(3)
    anime_viewer11.terminate()


if __name__ == "__main__":
    test_anime_viewer()
    test_obj()
