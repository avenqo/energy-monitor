// Number of 8x32 matrix elements
#define ROW_NUM 4
// number of pages
#define PAGE_NUM 3

const char* part_1[ROW_NUM][2] = {
  {"Energie", "[mode:right]"},
  {"[/home/haus/energy/consumption]", "[mode:left]"},
  {"Preis", ""},
  {"[/home/tibber/price/current]", ""}
};

const char* part_2[ROW_NUM][2] = {
  {"Tibber", "[mode:right]"},
  {"[/home/tibber/pulse/power/value/watt]", "[mode:left]"},
  {"DachPV", ""},
  {"[/home/haus/pv/dach]", ""}
};

const char* part_3[ROW_NUM][2] = {
  {"BATT", "[mode:right]"},
  {"[/home/victron/mp/soc]", "[mode:left]"},
  {"Victron", ""},
  {"[/home/victron/mp/power]", ""}
};

const char* (*page_data[PAGE_NUM])[2] = {part_1, part_2, part_3};

