"""
Generate triangles.txt
"""
import math

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
                faces.append((int(x), int(y), int(z)))
    print("{} vertices, {} faces in {}".format(
        len(vertices), len(faces), path
    ))
    return vertices, faces

def generate_triangles(template_path, mesh_path, result_path):
    old_verts, old_faces = get_obj(template_path)
    new_verts, new_faces = get_obj(mesh_path)
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
        for face in new_faces:
            x = index[face[0]-1]+1
            y = index[face[1]-1]+1
            z = index[face[2]-1]+1
            fp.write("%d %d %d\n" % (x,y,z))


if __name__ == "__main__":
    import sys
    generate_triangles(sys.argv[1], sys.argv[2], sys.argv[3])