right_limit = lambda a:f"REF=1270,392,10,200,\"./levels/level_{a}.txt\""
left_limit = lambda a:f"REF=0,392,10,200,\"./levels/level_{a}.txt\""

background_img = lambda a:f"BGI=\"./assets/map/{a}.png\",8"

level_name = lambda a:f"./level_{a}.txt"

map_count = 15

for i in range(map_count):
    content = [background_img(i)]
    if i > 0:
        content.append(left_limit(i - 1))
    if i < map_count - 1:
        content.append(right_limit(i + 1))
    file = open(level_name(i), "w")
    file.write("\n".join(content))
    file.close()