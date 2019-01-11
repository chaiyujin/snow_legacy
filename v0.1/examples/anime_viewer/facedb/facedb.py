"""
Generate triangles.txt
"""
import math

class Face:
    def __init__(self, x, y, z):
        if x.find("/") >= 0:
            v0, vt0, vn0 = x.split("/")
            v1, vt1, vn1 = y.split("/")
            v2, vt2, vn2 = z.split("/")
            self.v = [int(v0), int(v1), int(v2)]
            self.vt = [int(vt0), int(vt1), int(vt2)]
            self.vn = [int(vn0), int(vn1), int(vn2)]
        else:
            self.v = [int(x), int(y), int(z)]
            self.vt = [0, 0, 0]
            self.vn = [0, 0, 0]
    
    def equal(self, x, y, z):
        return self.v[0] == x and self.v[1] == y and self.v[2] == z


def get_obj(path):
    vertices = []
    vts = []
    faces = []
    with open(path) as fp:
        for line in fp:
            line = line.strip()
            if line[:2] == "v ":
                _, x, y, z = line.split()
                vertices.append((float(x), float(y), float(z)))
            elif line[:2] == "vt":
                _, x, y = line.split()
                vts.append((float(x), float(y)))
            elif line[:2] == "f ":
                _, x, y, z = line.split()
                faces.append(Face(x, y, z))

    print("{} vertices, {} faces in {}".format(
        len(vertices), len(faces), path
    ))
    return vertices, vts, faces

def generate_triangles(template_path, mesh_path, result_path):
    old_verts, vts, old_faces = get_obj(template_path)
    new_verts, _, new_faces = get_obj(mesh_path)
    
    def dist(p0, p1):
        return math.sqrt( (p0[0] - p1[0]) ** 2 + (p0[1] - p1[1]) ** 2 + (p0[2] - p1[2]) ** 2 )

    # get index
    index = []
    for count, vert in enumerate(new_verts):
        print(count, len(new_verts), end='\r')
        k = -1
        delta = 1e6
        for i, old in enumerate(old_verts):
            d = dist(vert, old)
            if d < delta:
                delta = d
                k = i
            if delta < 1e-6:
                break
        assert k >= 0
        index.append(k)

    with open(result_path, "w") as fp:
        fp.write("%d\n" % len(vts))
        for vt in vts:
            fp.write("%f %f\n" % vt)
        fp.write("%d\n" % len(new_faces))
        for face in new_faces:
            x = index[face.v[0]-1]+1
            y = index[face.v[1]-1]+1
            z = index[face.v[2]-1]+1
            find = False
            for f in old_faces:
                if f.equal(x, y, z):
                    find = True
                    fp.write("%d/%d/0 %d/%d/0 %d/%d/0\n" % (
                        f.v[0], f.vt[0],
                        f.v[1], f.vt[1],
                        f.v[2], f.vt[2]))
                    break
            assert find


if __name__ == "__main__":
    import sys
    generate_triangles(sys.argv[1], sys.argv[2], sys.argv[3])