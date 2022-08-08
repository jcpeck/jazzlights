#include "unisparks/board.h"

#if WEARABLE

#if ORANGE_VEST || GECKO_FOOT
#  include "unisparks/layouts/pixelmap.hpp"
#endif

#if CAMP_SIGN || IS_GUPPY || HAMMER || FAIRY_WAND
#  include "unisparks/layouts/reversemap.hpp"
#endif

#if IS_STAFF || ROPELIGHT
#  include "unisparks/layouts/matrix.hpp"
#endif

namespace unisparks {

namespace {

#if ORANGE_VEST

constexpr Point pixelMap[] = {
  { 0.0,  0.0}, { 0.0,  1.0}, { 0.0,  2.0}, { 0.0,  3.0}, { 0.0,  4.0}, { 0.0,  5.0}, { 0.0,  6.0}, { 0.0,  7.0},
  { 0.0,  8.0}, { 0.0,  9.0}, { 0.0, 10.0}, { 0.0, 11.0}, { 0.0, 12.0}, { 0.0, 13.0}, { 0.0, 14.0}, { 0.0, 15.0},
  { 0.0, 16.0}, { 0.0, 17.0}, { 0.5, 18.0}, { 1.0, 17.0}, { 1.0, 16.0}, { 1.0, 15.0}, { 1.0, 14.0}, { 1.0, 13.0},
  { 1.0, 12.0}, { 1.0, 11.0}, { 1.0, 10.0}, { 1.0,  9.0}, { 1.0,  8.0}, { 1.0,  7.0}, { 1.0,  6.0}, { 1.0,  5.0},
  { 1.0,  4.0}, { 1.0,  3.0}, { 1.0,  2.0}, { 1.0,  1.0}, { 1.5,  0.0}, { 2.0,  1.0}, { 2.0,  2.0}, { 2.0,  3.0},
  { 2.0,  4.0}, { 2.0,  5.0}, { 2.0,  6.0}, { 2.0,  7.0}, { 2.0,  8.0}, { 2.0,  9.0}, { 2.0, 10.0}, { 2.0, 11.0},
  { 2.0, 12.0}, { 2.0, 13.0}, { 2.0, 14.0}, { 2.0, 15.0}, { 2.0, 16.0}, { 2.0, 17.0}, { 2.5, 18.0}, { 3.0, 17.0},
  { 3.0, 16.0}, { 3.0, 15.0}, { 3.0, 14.0}, { 3.0, 13.0}, { 3.0, 12.0}, { 3.0, 11.0}, { 3.0, 10.0}, { 3.0,  9.0},
  { 3.0,  8.0}, { 3.0,  7.0}, { 3.0,  6.0}, { 3.0,  5.0}, { 3.0,  4.0}, { 3.0,  3.0}, { 3.0,  2.0}, { 3.0,  1.0},
  { 3.5,  0.0}, { 4.0,  1.0}, { 4.0,  2.0}, { 4.0,  3.0}, { 4.0,  4.0}, { 4.0,  5.0}, { 4.0,  6.0}, { 4.0,  7.0},
  { 4.0,  8.0}, { 4.0,  9.0}, { 4.0, 10.0}, { 4.0, 11.0}, { 4.0, 12.0}, { 4.0, 13.0}, { 4.0, 14.0}, { 4.0, 15.0},
  { 4.0, 16.0}, { 4.0, 17.0}, { 4.5, 18.0}, { 5.0, 17.0}, { 5.0, 16.0}, { 5.0, 15.0}, { 5.0, 14.0}, { 5.0, 13.0},
  { 5.0, 12.0}, { 5.0, 11.0}, { 5.0, 10.0}, { 5.0,  9.0}, { 5.0,  8.0}, { 5.0,  7.0}, { 5.0,  6.0}, { 5.0,  5.0},
  { 5.0,  4.0}, { 5.0,  3.0}, { 5.0,  2.0}, { 5.0,  1.0}, { 6.0,  0.0}, { 6.0,  1.0}, { 6.0,  2.0}, { 6.0,  3.0},
  { 6.0,  4.0}, { 6.0,  5.0}, { 6.0,  6.0}, { 6.0,  7.0}, { 6.0,  8.0}, { 6.0,  9.0}, { 6.0, 10.0}, { 6.0, 11.0},
  { 6.0, 12.0}, { 6.0, 13.0}, { 6.0, 14.0}, { 6.0, 15.0}, { 6.0, 16.0}, { 6.0, 17.0}, { 6.5, 18.0}, { 7.0, 17.0},
  { 7.0, 16.0}, { 7.0, 15.0}, { 7.0, 14.0}, { 7.0, 13.0}, { 7.0, 12.0}, { 7.0, 11.0}, { 7.0, 10.0}, { 7.0,  9.0},
  { 7.0,  8.0}, { 7.0,  7.0}, { 7.0,  6.0}, { 7.0,  5.0}, { 7.0,  4.0}, { 7.0,  3.0}, { 7.0,  2.0}, { 7.0,  1.0},
  { 7.5,  0.0}, { 8.0,  1.0}, { 8.0,  2.0}, { 8.0,  3.0}, { 8.0,  4.0}, { 8.0,  5.0}, { 8.0,  6.0}, { 8.0,  7.0},
  { 8.0,  8.0}, { 8.0,  9.0}, { 8.0, 10.0}, { 8.0, 11.0}, { 8.0, 12.0}, { 8.0, 13.0}, { 8.0, 14.0}, { 8.0, 15.0},
  { 8.0, 16.0}, { 8.0, 17.0}, { 8.5, 18.0}, { 9.0, 17.0}, { 9.0, 16.0}, { 9.0, 15.0}, { 9.0, 14.0}, { 9.0, 13.0},
  { 9.0, 12.0}, { 9.0, 11.0}, { 9.0, 10.0}, { 9.0,  9.0}, { 9.0,  8.0}, { 9.0,  7.0}, { 9.0,  6.0}, { 9.0,  5.0},
  { 9.0,  4.0}, { 9.0,  3.0}, { 9.0,  2.0}, { 9.0,  1.0}, { 9.5,  0.0}, {10.0,  1.0}, {10.0,  2.0}, {10.0,  3.0},
  {10.0,  4.0}, {10.0,  5.0}, {10.0,  6.0}, {10.0,  7.0}, {10.0,  8.0}, {10.0,  9.0}, {10.0, 10.0}, {10.0, 11.0},
  {10.0, 12.0}, {10.0, 13.0}, {10.0, 14.0}, {10.0, 15.0}, {10.0, 16.0}, {10.0, 17.0}, {10.5, 18.0}, {11.0, 17.0},
  {11.0, 16.0}, {11.0, 15.0}, {11.0, 14.0}, {11.0, 13.0}, {11.0, 12.0}, {11.0, 11.0}, {11.0, 10.0}, {11.0,  9.0},
  {11.0,  8.0}, {11.0,  7.0}, {11.0,  6.0}, {11.0,  5.0}, {11.0,  4.0}, {11.0,  3.0}, {11.0,  2.0}, {11.0,  1.0},
  {11.5,  0.0}, {12.0,  1.0}, {12.0,  2.0}, {12.0,  3.0}, {12.0,  4.0}, {12.0,  5.0}, {12.0,  6.0}, {12.0,  7.0},
  {12.0,  8.0}, {12.0,  9.0}, {12.0, 10.0}, {12.0, 11.0}, {12.0, 12.0}, {12.0, 13.0}, {12.0, 14.0}, {12.0, 15.0},
  {12.0, 16.0}, {12.0, 17.0}, {12.5, 18.0}, {13.0, 17.0}, {13.0, 16.0}, {13.0, 15.0}, {13.0, 14.0}, {13.0, 13.0},
  {13.0, 12.0}, {13.0, 11.0}, {13.0, 10.0}, {13.0,  9.0}, {13.0,  8.0}, {13.0,  7.0}, {13.0,  6.0}, {13.0,  5.0},
  {13.0,  4.0}, {13.0,  3.0}, {13.0,  2.0}, {13.0,  1.0}, {14.0,  0.0}, {14.0,  1.0}, {14.0,  2.0}, {14.0,  3.0},
  {14.0,  4.0}, {14.0,  5.0}, {14.0,  6.0}, {14.0,  7.0}, {14.0,  8.0}, {14.0,  9.0}, {14.0, 10.0}, {14.0, 11.0},
  {14.0, 12.0}, {14.0, 13.0}, {14.0, 14.0}, {14.0, 15.0}, {14.0, 16.0}, {14.0, 17.0}, {14.5, 18.0}, {15.0, 17.0},
  {15.0, 16.0}, {15.0, 15.0}, {15.0, 14.0}, {15.0, 13.0}, {15.0, 12.0}, {15.0, 11.0}, {15.0, 10.0}, {15.0,  9.0},
  {15.0,  8.0}, {15.0,  7.0}, {15.0,  6.0}, {15.0,  5.0}, {15.0,  4.0}, {15.0,  3.0}, {15.0,  2.0}, {15.0,  1.0},
  {15.5,  0.0}, {16.0,  1.0}, {16.0,  2.0}, {16.0,  3.0}, {16.0,  4.0}, {16.0,  5.0}, {16.0,  6.0}, {16.0,  7.0},
  {16.0,  8.0}, {16.0,  9.0}, {16.0, 10.0}, {16.0, 11.0}, {16.0, 12.0}, {16.0, 13.0}, {16.0, 14.0}, {16.0, 15.0},
  {16.0, 16.0}, {16.0, 17.0}, {16.5, 18.0}, {17.0, 17.0}, {17.0, 16.0}, {17.0, 15.0}, {17.0, 14.0}, {17.0, 13.0},
  {17.0, 12.0}, {17.0, 11.0}, {17.0, 10.0}, {17.0,  9.0}, {17.0,  8.0}, {17.0,  7.0}, {17.0,  6.0}, {17.0,  5.0},
  {17.0,  4.0}, {17.0,  3.0}, {17.0,  2.0}, {17.0,  1.0}, {17.5,  0.0}, {18.0,  1.0}, {18.0,  2.0}, {18.0,  3.0},
  {18.0,  4.0}, {18.0,  5.0}, {18.0,  6.0}, {18.0,  7.0}, {18.0,  8.0}, {18.0,  9.0}, {18.0, 10.0}, {18.0, 11.0},
  {18.0, 12.0}, {18.0, 13.0}, {18.0, 14.0}, {18.0, 15.0}, {18.0, 16.0}, {18.0, 17.0}, {18.5, 18.0}, {19.0, 17.0},
  {19.0, 16.0}, {19.0, 15.0}, {19.0, 14.0}, {19.0, 13.0}, {19.0, 12.0}, {19.0, 11.0}, {19.0, 10.0}, {19.0,  9.0},
  {19.0,  8.0}, {19.0,  7.0}, {19.0,  6.0}, {19.0,  5.0}, {19.0,  4.0}, {19.0,  3.0}, {19.0,  2.0}, {19.0,  1.0},
};

static_assert(LEDNUM == sizeof(pixelMap) / sizeof(pixelMap[0]), "bad LEDNUM");

PixelMap pixels(LEDNUM, pixelMap);

#endif // ORANGE_VEST

#if GECKO_FOOT

constexpr Point pixelMap[] = {
  {11.25, 0.00}, {11.25, 0.00}, {11.12, 0.00}, {11.12, 0.00}, {11.00, 0.00}, {11.00, 0.00}, {10.88, 0.00}, {10.88, 0.00},
  {10.75, 0.00}, {10.75, 0.00}, {10.62, 0.00}, {10.62, 0.00}, {10.50, 0.00}, {10.50, 0.00}, {10.50, 0.12}, {10.50, 0.12},
  {10.62, 0.12}, {10.62, 0.12}, {10.75, 0.12}, {10.75, 0.12}, {10.88, 0.12}, {10.88, 0.12}, {11.00, 0.12}, {11.00, 0.12},
  {11.12, 0.12}, {11.12, 0.12}, {11.25, 0.12}, {11.25, 0.12}, {11.25, 0.25}, {11.25, 0.25}, {11.12, 0.25}, {11.12, 0.25},
  {11.00, 0.25}, {11.00, 0.25}, {10.88, 0.25}, {10.88, 0.25}, {10.75, 0.25}, {10.75, 0.25}, {10.62, 0.25}, {10.62, 0.25},
  {10.50, 0.25}, {10.50, 0.25}, {10.50, 0.38}, {10.50, 0.38}, {10.62, 0.38}, {10.62, 0.38}, {10.75, 0.38}, {10.75, 0.38},
  {10.88, 0.38}, {10.88, 0.38}, {11.00, 0.38}, {11.00, 0.38}, {11.12, 0.38}, {11.12, 0.38}, {11.25, 0.38}, {11.25, 0.38},
  {11.25, 0.50}, {11.25, 0.50}, {11.12, 0.50}, {11.12, 0.50}, {11.00, 0.50}, {11.00, 0.50}, {10.88, 0.50}, {10.88, 0.50},
  {10.75, 0.50}, {10.75, 0.50}, {10.62, 0.50}, {10.62, 0.50}, {10.50, 0.50}, {10.50, 0.50}, {10.50, 0.62}, {10.50, 0.62},
  {10.62, 0.62}, {10.62, 0.62}, {10.75, 0.62}, {10.75, 0.62}, {10.88, 0.62}, {10.88, 0.62}, {11.00, 0.62}, {11.00, 0.62},
  {11.12, 0.62}, {11.12, 0.62}, {11.25, 0.62}, {11.25, 0.62}, {11.25, 0.75}, {11.25, 0.75}, {11.12, 0.75}, {11.12, 0.75},
  {11.00, 0.75}, {11.00, 0.75}, {10.88, 0.75}, {10.88, 0.75}, {10.75, 0.75}, {10.75, 0.75}, {10.62, 0.75}, {10.62, 0.75},
  {10.50, 0.75}, {10.50, 0.75}, {-5.95, -6.45}, {-5.95, 2.93}, {19.70, -6.45}, {19.70, 2.93},
};

static_assert(LEDNUM == sizeof(pixelMap) / sizeof(pixelMap[0]), "bad LEDNUM");

PixelMap pixels(LEDNUM, pixelMap);

#endif  // GECKO_FOOT

#if CAMP_SIGN

constexpr int pixelMap[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
     30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
     60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
     90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
    120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,
    150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,
    210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
    270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,
    300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,324,325,326,327,328,329,
    330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,353,354,355,356,357,358,359,
    360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,384,385,386,387,388,389,
    390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,417,418,419,
    420,421,422,423,424,425,426,427,428,429,430,431,432,433,434,435,436,437,438,439,440,441,442,443,444,445,446,447,448,449,
    450,451,452,453,454,455,456,457,458,459,460,461,462,463,464,465,466,467,468,469,470,471,472,473,474,475,476,477,478,479,
    480,481,482,483,484,485,486,487,488,489,490,491,492,493,494,495,496,497,498,499,500,501,502,503,504,505,506,507,508,509,
    510,511,512,513,514,515,516,517,518,519,520,521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,
    540,541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560,561,562,563,564,565,566,567,568,569,
    570,571,572,573,574,575,576,577,578,579,580,581,582,583,584,585,586,587,588,589,590,591,592,593,594,595,596,597,598,599,
    600,601,602,603,604,605,606,607,608,609,610,611,612,613,614,615,616,617,618,619,620,621,622,623,624,625,626,627,628,629,
    630,631,632,633,634,635,636,637,638,639,640,641,642,643,644,645,646,647,648,649,650,651,652,653,654,655,656,657,658,659,
    660,661,662,663,664,665,666,667,668,669,670,671,672,673,674,675,676,677,678,679,680,681,682,683,684,685,686,687,688,689,
    690,691,692,693,694,695,696,697,698,699,700,701,702,703,704,705,706,707,708,709,710,711,712,713,714,715,716,717,718,719,
    720,721,722,723,724,725,726,727,728,729,730,731,732,733,734,735,736,737,738,739,740,741,742,743,744,745,746,747,748,749,
    750,751,752,753,754,755,756,757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772,773,774,775,776,777,778,779,
    780,781,782,783,784,785,786,787,788,789,790,791,792,793,794,795,796,797,798,799,800,801,802,803,804,805,806,807,808,809,
    810,811,812,813,814,815,816,817,818,819,820,821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,836,837,838,839,
    840,841,842,843,844,845,846,847,848,849,850,851,852,853,854,855,856,857,858,859,860,861,862,863,864,865,866,867,868,869,
    870,871,872,873,874,875,876,877,878,879,880,881,882,883,884,885,886,887,888,889,890,891,892,893,894,895,896,897,898,899
};

ReverseMap<LEDNUM> pixels(pixelMap, /*MATRIX_WIDTH=*/30, /*MATRIX_HEIGHT=*/30);

#endif // CAMP_SIGN

#if IS_GUPPY

constexpr int pixelMap[] = {
   215,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,216,217,218,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,219,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,220,
   222,214, 39, 40, 41, 42, 43, 44, 45, 46, 47,208,209,210,221,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,223,
    62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,224,
    76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,225,
    90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,226,
   104,105,106,107,108,109,110,111,112,113,114,115,116,117,227,
   118,119,120,121,122,123,124,125,126,127,128,129,130,131,228,
   132,133,134,135,136,137,138,139,140,141,142,143,144,145,229,
   146,147,148,149,150,151,152,153,154,155,156,157,158,159,230,
   232,239,160,161,162,163,164,165,166,167,168,211,212,213,231,
   169,170,171,172,173,174,175,176,177,178,179,180,181,182,233,
   183,184,185,186,187,188,189,190,191,192,193,194,195,196,234,
   235,197,198,199,200,201,202,203,204,205,206,207,236,237,238,
   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,
   255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
   270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,
   285,286,287,288,289,270,271,272,273,274,275,276,277,278,279,
};

ReverseMap<LEDNUM> pixels(pixelMap, /*MATRIX_WIDTH=*/15, /*MATRIX_HEIGHT=*/20);

#endif // IS_GUPPY

#if HAMMER

constexpr int pixelMap[] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
};

ReverseMap<LEDNUM> pixels(pixelMap, /*MATRIX_WIDTH=*/1, /*MATRIX_HEIGHT=*/20);

#endif // HAMMER

#if FAIRY_WAND

constexpr int pixelMap[] = {
  0, 1, 2,
  3, 4, 5,
  6, 7, 8,
};

static_assert(LEDNUM == sizeof(pixelMap) / sizeof(pixelMap[0]), "bad LEDNUM");

ReverseMap<LEDNUM> pixels(pixelMap, /*MATRIX_WIDTH=*/3, /*MATRIX_HEIGHT=*/3);

#endif // FAIRY_WAND

#if IS_STAFF
Matrix pixels(/*w=*/1, /*h=*/LEDNUM);
#endif  // IS_STAFF

#if ROPELIGHT
Matrix pixels(/*w=*/LEDNUM, /*h=*/1);
#endif  // ROPELIGHT

}  // namespace

const Layout* GetLayout() {
  return &pixels;
}

}  // namespace unisparks

#endif  // WEARABLE
