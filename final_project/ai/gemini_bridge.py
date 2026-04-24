import argparse
import sys
import os
from datetime import datetime
from google import genai
from google.genai import types
from google.genai import errors

FULL_MODEL_LIST = [
    "gemini-2.5-flash",
    "gemini-2.5-flash-lite",
]

def build_prompt(args):
    if args.type == "global_biomes":
        return f"""
Generate exactly 25 biome cells for a shared 5x5 km world grid used by a zombie apocalypse travel game.

Return only the exact output format below.
Do not use markdown.
Do not explain anything.
Each line must be:
x y biome

Biome numbers:
0 unknown
1 water
2 river
3 urban
4 suburban
5 industrial
6 farmland
7 wilderness
8 road
9 landmark

Use normal north-west map orientation.
Do not rotate the grid based on player direction.
Lower y is north.
Higher y is south.
Lower x is west.
Higher x is east.

Generate a 5 by 5 square centered on:
Grid x: {args.grid_x}
Grid y: {args.grid_y}

Player current grid:
Player x: {args.player_grid_x}
Player y: {args.player_grid_y}

The player is facing:
{args.direction}

If facing west, the player should be near the middle of the left edge of the generated 5x5.
If facing east, the player should be near the middle of the right edge of the generated 5x5.
If facing north, the player should be near the middle of the top edge of the generated 5x5.
If facing south, the player should be near the middle of the bottom edge of the generated 5x5.

Center latitude: {args.latitude}
Center longitude: {args.longitude}

Use plausible real-world geography and settlement patterns.
Urban near dense cities.
Suburban around urban.
Industrial near highways, ports, rail, airports, warehouses, and city edges.
Farmland outside dense cities.
Wilderness in rural, forest, mountain, desert, or undeveloped areas.
Road for major highway corridors.
Water is only for full lake or ocean cells where most of the entire 5x5 km cell is open water.
Do not use water for ordinary city blocks, canals, ponds, waterfront edges, marinas, drainage basins, or small lakes.
River is only for a major river crossing or a cell dominated by a wide river corridor.
Never surround the player with water.
Never place water directly adjacent to the player grid unless the coordinate is clearly inside a large lake or ocean.
For player spawn areas near Chicago, New York, or Los Angeles, keep the immediate playable cells passable.
Chicago should not be surrounded by water. Lake Michigan should only appear east/northeast of Chicago, not on every side.
New York may have water nearby, but not as a full ring around the player.
Los Angeles should not generate water except along the coast, generally west/southwest of the basin.
At least 20 of the 25 generated cells must be passable biomes: urban, suburban, industrial, farmland, wilderness, road, or landmark.
Landmark should be rare.

Format:
BEGIN_GLOBAL_BIOMES
x y biome
END_GLOBAL_BIOMES
"""

    if args.type == "biome_texts":
        return f"""
Generate travel text entries for a zombie apocalypse survival travel game set around the 2012 era.

Return only the exact output format below.
Do not use markdown.
Do not explain anything.
Each line must be no more than {args.text_limit} characters.
Generate multiple varied lines per biome.
Use grounded, physical, observable details.
Use 2012-era world details when useful: strip malls, old smartphones, gas stations, GPS units, highway signs, dead suburbs, chain stores, office parks, box trucks, foreclosed houses, power substations, construction zones, farms, wooded roads.
Avoid fantasy language.
Avoid using the # character.
Avoid using the | character except between biome number and text.

Biome numbers:
0 unknown
1 water
2 river
3 urban
4 suburban
5 industrial
6 farmland
7 wilderness
8 road
9 landmark

Format:
BEGIN_BIOME_TEXTS
biomeNumber|travel text
END_BIOME_TEXTS

Examples:
BEGIN_BIOME_TEXTS
3|You pass dead storefronts with sun-faded sale posters still taped behind the glass.
4|You move past quiet driveways where old sedans sit under coats of pollen and dust.
5|You follow a service road beside fenced warehouses and loading bays left half open.
6|You cross flat farm roads between empty fields and irrigation rigs ticking in the wind.
7|Branches drag against your sleeves as the road thins into broken asphalt and trees.
8|You follow a cracked highway shoulder past stalled cars and green signs bleached pale.
END_BIOME_TEXTS

Generate:
{args.biome_text_count} entries for each requested biome.

Requested biomes:
{args.biomes}
"""

    if args.type == "encounters":
        return f"""
Generate travel text entries for a zombie apocalypse survival travel game set around the 2012 era.

Return only the exact output format below.
Do not use markdown.
Do not explain anything.
Each line must be no more than {args.text_limit} characters.
Generate a decent amount of varied lines every time.
Use grounded, physical, observable details.
Use 2012-era world details when useful: strip malls, old smartphones, gas stations, GPS units, highway signs, dead suburbs, chain stores, office parks, box trucks, foreclosed houses, power substations, construction zones, farms, wooded roads.
Avoid fantasy language.
Avoid using the # character.
Avoid using the | character except between biome number and text.

Biome numbers:
2 river
3 urban
4 suburban
5 industrial
6 farmland
7 wilderness
8 road
9 landmark

Do not generate water entries.

Format:
BEGIN_BIOME_TEXTS
biomeNumber|travel text
END_BIOME_TEXTS

Examples:
BEGIN_BIOME_TEXTS
3|You move through dense urban blocks, passing dark storefronts, dead signals, and streets packed with abandoned cars.
3|You take another route through the city, keeping close to walls and broken glass as the intersections open ahead.
4|You cross quiet suburbs where lawns have gone wild and houses sit with their doors half-open.
4|You move through a suburban stretch of cul-de-sacs, stalled cars, and empty windows.
5|You pass through industrial ground, between warehouses, fenced yards, loading docks, and rusting equipment.
5|You follow the edge of an industrial district where the road smells of oil, rainwater, and old smoke.
6|You travel past farmland, long fields, collapsed barns, and fence lines bending into the grass.
6|You move along the edge of open fields, watching crows lift from the ruined crops ahead.
7|You cut through wilderness, where the road narrows and the trees crowd close on both sides.
7|You move through thick wilderness, listening to branches scrape and shift beyond the trail.
8|You follow a road corridor, cracked but still clear enough to make steady progress.
8|You stay on the road, passing mile markers and vehicles left behind in a hurry.
9|You approach a strange landmark, something marked by old barricades and signs no one has touched in years.
9|You pass through a place that feels marked, as if people once gathered here before everything went quiet.
2|You follow a river crossing, watching the water move darkly beneath ruined bridgework.
END_BIOME_TEXTS

Generate at least {args.count} entries for each biome.
Current biome context: {args.biome}
Day: {args.day}
Hour: {args.hour}
"""
    if args.type == "loot_table":
        return f"""
Generate new loot entries for a zombie apocalypse survival game set around the 2012 era.

Return only the exact output format below.
Use underscores instead of spaces in item names.
Do not use commas.
Do not use markdown.
Do not explain anything outside the sections.
Each item name must be no more than {args.name_limit} characters.
Be creative and somewhat detailed.
Use 2012-era brands, mall items, hardware store items, police/security gear, camping gear, sporting goods, old electronics, and worn civilian objects.
Branded/special items are allowed, but keep them grounded and believable.

Armor format:
name protection mobility
Protection is 1 to 8.
Mobility is 1 to 5.
Higher protection usually means lower mobility.

Melee format:
name damage speed
Damage is 1 to 8.
Speed is 1 to 5.
Heavy weapons usually hit harder but slower.

Firearm format:
name damage speed
Damage is 2 to 10.
Speed is 1 to 5.
Rare or special firearms can be stronger, but should still feel grounded.

Flashlight format:
name radius range
Radius is 20 to 90.
Range is 1 to 8.
Lantern-style lights have wider radius and lower range.
Directional flashlights have smaller radius and higher range.

Examples:
BEGIN_ARMOR
Mall_Security_Vest 3 3
Paintball_Chest_Rig 2 4
END_ARMOR

BEGIN_MELEE
Stanley_Crowbar 3 3
Aluminum_Baseball_Bat 4 3
END_MELEE

BEGIN_FIREARM
Ruger_10_22 3 4
Mossberg_500 7 2
END_FIREARM

BEGIN_FLASHLIGHT
Maglite_3D 45 5
Coleman_LED_Lantern 80 2
END_FLASHLIGHT

Generate:
{args.armor_count} armor entries
{args.melee_count} melee entries
{args.firearm_count} firearm entries
{args.flashlight_count} flashlight entries
"""

    if args.type == "loot_maps":
        return f"""
Generate loot map files for a Raylib zombie apocalypse looting game.

Return only the exact format below.
Do not use markdown.
Do not explain anything.
Use only tile numbers separated by spaces.
Use underscores in filenames.
Every map must be rectangular.
Every row in a map must have exactly {args.map_width} numbers.
Every map must have exactly {args.map_height} rows.

Tile meanings:
0 = walkable floor, hallway, room interior, spawnable space
4 = wall, border, shelf, counter, obstacle, blocked structure
5 = exit button marker

Use only these tile numbers:
0
4
5

Each map must include exactly one tile 5.
The outer border should mostly be 4.
The exit button marker 5 should usually be on or near an outer border.
There must be connected 0 paths from the player area to the exit.
There must be enough 0 floor space for loot and zombie spawning.
Do not make empty open boxes.
Use 4 to create rooms, corridors, shelves, counters, blocked aisles, storage rooms, offices, chokepoints, and maze-like looting paths.
Use the example style: dense 4 walls and structures, with 0 paths and rooms carved through them.
The filename must start with this exact prefix: {args.prefix}
Use this biome style: {args.biome}

Requested map count: {args.map_count}

Format:
BEGIN_MAP filename.txt
4 4 4 4 4 4 4 4
4 0 0 0 4 0 0 4
4 0 4 0 4 0 0 4
4 0 4 0 0 0 0 4
4 0 4 4 4 4 0 4
4 0 0 0 0 0 5 4
4 4 4 4 4 4 4 4
END_MAP
"""

    return ""

def write_output(output_file, text):
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(text.strip())

# troubleshooting logs
def append_ai_debug_log(output_file, text):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    with open(output_file, "a", encoding="utf-8") as f:
        f.write(f"\n[{timestamp}] {text}\n")

def call_gemini(api_key, prompt_type, prompt, output_file, debug_output_file, use_search, args):
    api_key = api_key.strip(' "\'\n\r')

    if not api_key:
        write_output(output_file, "")
        sys.exit(1)

    client = genai.Client(api_key=api_key)

    for model_name in FULL_MODEL_LIST:
        config_kwargs = {}

        if use_search:
            config_kwargs["tools"] = [types.Tool(google_search=types.GoogleSearch())]

        append_ai_debug_log(debug_output_file, f"{prompt_type} called {model_name}")

        try:
            response = client.models.generate_content(
                model=model_name,
                contents=prompt,
                config=types.GenerateContentConfig(**config_kwargs) if config_kwargs else None
            )

            if response.text:
                if prompt_type == "loot_table":
                    result = append_loot_output(response.text)
                    write_output(output_file, result)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                elif prompt_type == "biome_texts":
                    result = append_biome_text_output(response.text)
                    write_output(output_file, result)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                elif prompt_type == "global_biomes":
                    result = append_global_biome_output(response.text, int(args.player_grid_x), int(args.player_grid_y))
                    write_output(output_file, result)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                elif prompt_type == "loot_maps":
                    result = append_loot_map_output(response.text)
                    write_output(output_file, result)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                elif prompt_type == "encounters":
                    result = append_encounter_output(response.text)
                    write_output(output_file, result)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                else:
                    write_output(output_file, response.text)
                    append_ai_debug_log(debug_output_file, f"{prompt_type} output\n{response.text.strip()}")
                return


        except errors.APIError as e:
            append_ai_debug_log(debug_output_file, f"{prompt_type} error {model_name} {e}")
            continue

    write_output(output_file, "")
    append_ai_debug_log(debug_output_file, f"{prompt_type} error all_models_failed")
    sys.exit(1)



def read_section(text, start_marker, end_marker):
    lines = text.splitlines()
    output = []
    active = False

    for line in lines:
        line = line.strip()

        if line == start_marker:
            active = True
            continue

        if line == end_marker:
            active = False
            continue

        if active and line:
            output.append(line)

    return output

# // post processing since we dont fully trust ai output
# // validate
def valid_loot_line(line):
    parts = line.split()

    if len(parts) != 3:
        return False

    if not parts[1].isdigit():
        return False

    if not parts[2].isdigit():
        return False

    return True

def append_lines(file_path, lines):
    valid_lines = []

    for line in lines:
        if valid_loot_line(line):
            valid_lines.append(line)

    if not valid_lines:
        return 0

    with open(file_path, "a", encoding="utf-8") as f:
        for line in valid_lines:
            f.write(line + "\n")

    return len(valid_lines)

def append_loot_output(text):
    armor = read_section(text, "BEGIN_ARMOR", "END_ARMOR")
    melee = read_section(text, "BEGIN_MELEE", "END_MELEE")
    firearm = read_section(text, "BEGIN_FIREARM", "END_FIREARM")
    flashlight = read_section(text, "BEGIN_FLASHLIGHT", "END_FLASHLIGHT")

    armor_count = append_lines("loot/armor.txt", armor)
    melee_count = append_lines("loot/melee.txt", melee)
    firearm_count = append_lines("loot/firearm.txt", firearm)
    flashlight_count = append_lines("loot/flashlight.txt", flashlight)

    return f"armor {armor_count}\nmelee {melee_count}\nfirearm {firearm_count}\nflashlight {flashlight_count}"

def valid_biome_text_line(line):
    if "|" not in line:
        return False

    parts = line.split("|", 1)

    if len(parts) != 2:
        return False

    if not parts[0].isdigit():
        return False

    biome = int(parts[0])

    if biome < 0 or biome > 9:
        return False

    if not parts[1].strip():
        return False

    if "#" in parts[1]:
        return False

    return True

def append_biome_text_output(text):
    lines = read_section(text, "BEGIN_BIOME_TEXTS", "END_BIOME_TEXTS")
    valid_lines = []

    for line in lines:
        line = line.strip()

        if valid_biome_text_line(line):
            valid_lines.append(line)

    if not valid_lines:
        return "biome_texts 0"

    with open("data/biome_travel_texts.txt", "a", encoding="utf-8") as f:
        for line in valid_lines:
            f.write(line + "\n")

    return f"biome_texts {len(valid_lines)}"

def valid_global_biome_line(line):
    parts = line.split()

    if len(parts) != 3:
        return False

    try:
        x = int(parts[0])
        y = int(parts[1])
        biome = int(parts[2])
    except ValueError:
        return False

    if biome < 0 or biome > 9:
        return False

    return True

def is_passable_biome(biome):
    return biome not in [1]

def append_global_biome_output(text, player_grid_x, player_grid_y):
    lines = read_section(text, "BEGIN_GLOBAL_BIOMES", "END_GLOBAL_BIOMES")
    cells = []

    for line in lines:
        line = line.strip()

        if valid_global_biome_line(line):
            parts = line.split()
            cells.append((int(parts[0]), int(parts[1]), int(parts[2])))

    if not cells:
        return "global_biomes 0"

    water_count = 0

    for x, y, biome in cells:
        if biome == 1:
            water_count += 1

    valid_lines = []

    for x, y, biome in cells:
        adjacent_to_player = abs(x - player_grid_x) <= 1 and abs(y - player_grid_y) <= 1

        if biome == 1 and adjacent_to_player:
            biome = 4

        if biome == 1 and water_count > 5:
            biome = 7

        valid_lines.append(f"{x} {y} {biome}")

    with open("data/global_biomes.txt", "a", encoding="utf-8") as f:
        for line in valid_lines:
            f.write(line + "\n")

    return f"global_biomes {len(valid_lines)}"

def clean_map_filename(filename):
    filename = filename.strip()
    filename = filename.replace("\\", "")
    filename = filename.replace("/", "")
    filename = filename.replace("..", "")
    filename = filename.replace(" ", "_")

    if not filename.endswith(".txt"):
        filename += ".txt"

    return filename

def valid_map_row(line):
    parts = line.split()

    if not parts:
        return False

    for part in parts:
        if part not in ["0", "4", "5"]:
            return False

    return True

def append_loot_map_output(text):
    os.makedirs("maps", exist_ok=True)

    lines = text.splitlines()
    active = False
    filename = ""
    rows = []
    created = 0

    for raw_line in lines:
        line = raw_line.strip()

        if line.startswith("BEGIN_MAP"):
            parts = line.split()

            if len(parts) >= 2:
                filename = clean_map_filename(parts[1])
                rows = []
                active = True

            continue

        if line == "END_MAP":
            if active and filename and rows:
                width = len(rows[0].split())
                valid = True
                exit_count = 0
                floor_count = 0

                for row in rows:
                    row_parts = row.split()

                    if len(row_parts) != width:
                        valid = False

                    for part in row_parts:
                        if part == "5":
                            exit_count += 1
                        if part == "0":
                            floor_count += 1

                if valid and exit_count == 1 and floor_count >= 20:
                    with open(os.path.join("maps", filename), "w", encoding="utf-8") as f:
                        for row in rows:
                            f.write(row + "\n")

                    created += 1

            active = False
            filename = ""
            rows = []
            continue

        if active and valid_map_row(line):
            rows.append(line)

    return f"loot_maps {created}"

def valid_encounter_line(line):
    parts = line.split("|")

    if len(parts) != 4:
        return False

    if not parts[1].isdigit():
        return False

    if "#" in line:
        return False

    return True

def append_encounter_output(text):
    lines = read_section(text, "BEGIN_BIOME_TEXTS", "END_BIOME_TEXTS")
    valid_lines = []

    for line in lines:
        line = line.strip()

        if valid_biome_text_line(line):
            parts = line.split("|", 1)
            biome = int(parts[0])

            if biome != 1:
                valid_lines.append(line)

    if not valid_lines:
        return "biome_texts 0"

    with open("data/biome_travel_texts.txt", "a", encoding="utf-8") as f:
        for line in valid_lines:
            f.write(line + "\n")

    return f"biome_texts {len(valid_lines)}"

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    # there's a bunch of redundant ones here since i keep changing prompts
    parser.add_argument("--key", required=True)
    parser.add_argument("--type", required=True)
    parser.add_argument("--output", default="data/ai_output.txt")
    parser.add_argument("--debug-output", default="data/ai_debug_log.txt")

    parser.add_argument("--search", action="store_true")

    parser.add_argument("--grid-x", default="0")
    parser.add_argument("--grid-y", default="0")
    parser.add_argument("--start-latitude", default="0")
    parser.add_argument("--start-longitude", default="0")
    parser.add_argument("--latitude", default="0")
    parser.add_argument("--longitude", default="0")
    parser.add_argument("--start-city", default="")

    parser.add_argument("--armor-count", default="0")
    parser.add_argument("--melee-count", default="0")
    parser.add_argument("--firearm-count", default="0")
    parser.add_argument("--flashlight-count", default="0")
    parser.add_argument("--name-limit", default="32")

    parser.add_argument("--biome-text-count", default="0")
    parser.add_argument("--biomes", default="3,4,5,6,7,8")
    parser.add_argument("--text-limit", default="140")

    parser.add_argument("--map-count", default="1")
    parser.add_argument("--map-width", default="40")
    parser.add_argument("--map-height", default="24")

    parser.add_argument("--prefix", default="")

    parser.add_argument("--player-grid-x", default="0")
    parser.add_argument("--player-grid-y", default="0")
    parser.add_argument("--direction", default="")

    parser.add_argument("--radius-cells", default="2")
    parser.add_argument("--cell-count", default="25")

    parser.add_argument("--biome", default="")
    parser.add_argument("--day", default="0")
    parser.add_argument("--hour", default="0")
    parser.add_argument("--count", default="4")

    args = parser.parse_args()

    prompt = build_prompt(args)

    if not prompt:
        write_output(args.output, "")
        sys.exit(1)

    call_gemini(args.key, args.type, prompt, args.output, args.debug_output, args.search, args)