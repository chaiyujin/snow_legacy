import os

class Face(object):
    def __init__(self, v, vt=[0,0,0], vn=[0,0,0]):
        self.v = v
        self.vt = vt
        self.vn = vn


def read_obj(path):
    verts = []
    texts = []
    faces = []
    mapping = {}
    def add_mapping(v, vt):
        v = int(v)
        vt = int(vt)
        if v in mapping and mapping[v] != vt:
            vti = vt
            vtj = mapping[v]
            print(texts[vti])
            print(texts[vtj])
            raise ValueError("{}: {} != {}".format(v, mapping[v], vt))
        else:
            mapping[v] = vt
    with open(path) as fp:
        for line in fp:
            line = line.strip()
            if line[:2] == "v ":
                _, x, y, z = line.split()
                x, y, z = float(x), float(y), float(z)
                verts.append((x,y,z))
            elif line[:2] == "vt":
                _, x, y = line.split()
                x, y = float(x), float(y)
                texts.append((x, y))
            elif line[:2] == "f ":
                line = line.split()
                v0, vt0, vn0 = line[1].split("/")
                v1, vt1, vn1 = line[2].split("/")
                v2, vt2, vn2 = line[3].split("/")
                add_mapping(v0, vt0)
                add_mapping(v1, vt1)
                add_mapping(v2, vt2)
                faces.append(Face(
                    (v0,v1,v2),
                    (vt0,vt1,vt2),
                    (vn0,vn1,vn2)
                ))
                if len(line) == 5:
                    v3, vt3, vn3 = line[4].split("/")
                    add_mapping(v3, vt3)
                    faces.append(Face(
                        (v0, v2, v3),
                        (vt0,vt2,vt3),
                        (vn0,vn2,vn3)
                    ))
    return verts, texts, faces


def write_obj(path, verts, texts, faces):
    with open(path, "w") as fp:
        pass

if __name__ == "__main__":
    verts, texts, faces = read_obj("template.obj")