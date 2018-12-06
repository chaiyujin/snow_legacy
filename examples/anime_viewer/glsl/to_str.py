import os

def to_str(path):
    lines = []
    with open(path) as fp:
        for line in fp:
            line = line.strip()
            if line.find("//") >= 0:
                line = line[:line.find("//")]
            if len(line) > 0:
                lines.append(line)
    max_len = max([len(line) for line in lines])
    with open(os.path.splitext(path)[0] + ".txt", "w") as fp:
        for line in lines[1:]:
            fp.write("\"{}\"\n".format(line))

if __name__ == "__main__":
    to_str("frag.glsl")
    to_str("vert.glsl")
    