#include "figlet_font.h"

namespace Figlet {

static char const Hardblank = '$';
static unsigned const FontHeight = 8;
static unsigned const FontMaxLen = 4;

// clang-format off

static FontFiglet characters[] = {

	// letter "space"
	{ 32,
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" $",
			" $",
			" $",
			" $",
			" $",
			" $",
			" $",
			" $" } },

	// letter N. 33 " ! "
	{ 33,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" ",
			"[",
			"[",
			"[",
			" ",
			"[",
			" ",
			" " } },

	// letter N. 34 " " "
	{ 34,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[[",
			"[[",
			"  ",
			"  ",
			"  ",
			"  ",
			"  ",
			"  " } },

	// letter N. 35 " # "
	{ 35,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"]] ",
			"##[",
			"]] ",
			"##[",
			"]] ",
			"   ",
			"   " } },

	// letter N. 36 " $ "
	{ 36,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"] ",
			"#[",
			"# ",
			"][",
			"#[",
			"] ",
			"  " } },

	// letter N. 37 " % "
	{ 37,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			" [",
			"] ",
			"[ ",
			"[[",
			"  ",
			"  " } },

	// letter N. 38 " & "
	{ 38,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"] ",
			"[[",
			"] ",
			"[[",
			"][",
			"  ",
			"  " } },

	// letter N. 39 " ' "
	{ 39,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[",
			"[",
			" ",
			" ",
			" ",
			" ",
			" ",
			" " } },

	// letter N. 40 " ( "
	{ 40,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"] ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"] ",
			"  " } },

	// letter N. 41 " ) "
	{ 41,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"[ ",
			"] ",
			"] ",
			"] ",
			"] ",
			"] ",
			"[ ",
			"  " } },

	// letter N. 42 " * "
	{ 42,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"] ",
			"#[",
			"] ",
			"[[",
			"  ",
			"  " } },

	// letter N. 43 " + "
	{ 43,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"] ",
			"#[",
			"] ",
			"  ",
			"  ",
			"  " } },

	// letter N. 44 " , "
	{ 44,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"  ",
			"  ",
			"  ",
			"  ",
			"  ",
			"] ",
			"[ ",
			"  " } },

	// letter N. 45 " - "
	{ 45,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"  ",
			"#[",
			"  ",
			"  ",
			"  ",
			"  " } },

	// letter N. 46 " . "
	{ 46,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" ",
			" ",
			" ",
			" ",
			" ",
			"[",
			" ",
			" " } },

	// letter N. 47 " / "
	{ 47,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" [",
			" [",
			"] ",
			"] ",
			"] ",
			"[ ",
			"[ ",
			"  " } },

	// letter N. 48 " 0 "
	{ 48,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"#[",
			"[[",
			"#[",
			"#[",
			"  ",
			"  " } },

	// letter N. 49 " 1 "
	{ 49,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"] ",
			"# ",
			"] ",
			"] ",
			"#[",
			"  ",
			"  " } },

	// letter N. 50 " 2 "
	{ 50,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			" [",
			"#[",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 51 " 3 "
	{ 51,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			" [",
			"#[",
			" [",
			"#[",
			"  ",
			"  " } },

	// letter N. 52 " 4 "
	{ 52,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"#[",
			" [",
			" [",
			"  ",
			"  " } },

	// letter N. 53 " 5 "
	{ 53,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"#[",
			" [",
			"#[",
			"  ",
			"  " } },

	// letter N. 54 " 6 "
	{ 54,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"#[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 55 " 7 "
	{ 55,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			" [",
			" [",
			" [",
			" [",
			"  ",
			"  " } },

	// letter N. 56 " 8 "
	{ 56,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"#[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 57 " 9 "
	{ 57,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"#[",
			" [",
			"#[",
			"  ",
			"  " } },

	// letter N. 58 " : "
	{ 58,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" ",
			" ",
			" ",
			"[",
			" ",
			"[",
			" ",
			" " } },

	// letter N. 59 " ; "
	{ 59,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"  ",
			"  ",
			"  ",
			"] ",
			"  ",
			"] ",
			"[ ",
			"  " } },

	// letter N. 60 " < "
	{ 60,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			" [",
			"] ",
			"[ ",
			"] ",
			" [",
			"  ",
			"  " } },

	// letter N. 61 " = "
	{ 61,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"  ",
			"#[",
			"  ",
			"  ",
			"  " } },

	// letter N. 62 " > "
	{ 62,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[ ",
			"] ",
			" [",
			"] ",
			"[ ",
			"  ",
			"  " } },

	// letter N. 63 " ? "
	{ 63,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			" [",
			"] ",
			"  ",
			"] ",
			"  ",
			"  " } },

	// letter N. 64 " @ "
	{ 64,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"    ",
			"]#[ ",
			"[ ] ",
			"[#] ",
			"[## ",
			"[   ",
			"]#  ",
			"    " } },

	// letter N. 65 " A "
	{ 65,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			"[[",
			"  ",
			"  " } },

	// letter N. 66 " B "
	{ 66,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"# ",
			"[[",
			"# ",
			"[[",
			"# ",
			"  ",
			"  " } },

	// letter N. 67 " C "
	{ 67,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"[ ",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 68 " D "
	{ 68,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"# ",
			"[[",
			"[[",
			"[[",
			"# ",
			"  ",
			"  " } },

	// letter N. 69 " E "
	{ 69,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"#[",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 70 " F "
	{ 70,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"#[",
			"[ ",
			"[ ",
			"  ",
			"  " } },

	// letter N. 71 " G "
	{ 71,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"[ ",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 72 " H "
	{ 72,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"#[",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 73 " I "
	{ 73,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"] ",
			"] ",
			"] ",
			"#[",
			"  ",
			"  " } },

	// letter N. 74 " J "
	{ 74,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			" [",
			" [",
			" [",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 75 " K "
	{ 75,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"# ",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 76 " L "
	{ 76,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 77 " M "
	{ 77,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"[ [",
			"#][",
			"[ [",
			"[ [",
			"[ [",
			"   ",
			"   " } },

	// letter N. 78 " N "
	{ 78,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"[ [",
			"# [",
			"[ [",
			"[][",
			"[ [",
			"   ",
			"   " } },

	// letter N. 79 " O "
	{ 79,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 80 " P "
	{ 80,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"#[",
			"[ ",
			"[ ",
			"  ",
			"  " } },

	// letter N. 81 " Q "
	{ 81,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			"#[",
			" [",
			"  " } },

	// letter N. 82 " R "
	{ 82,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[[",
			"# ",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 83 " S "
	{ 83,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"[ ",
			"] ",
			" [",
			"#[",
			"  ",
			"  " } },

	// letter N. 84 " T "
	{ 84,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			"] ",
			"] ",
			"] ",
			"] ",
			"  ",
			"  " } },

	// letter N. 85 " U "
	{ 85,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 86 " V "
	{ 86,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"[[",
			"[[",
			"] ",
			"  ",
			"  " } },

	// letter N. 87 " W "
	{ 87,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"[ [",
			"[ [",
			"[ [",
			"#][",
			"[ [",
			"   ",
			"   " } },

	// letter N. 88 " X "
	{ 88,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"] ",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 89 " Y "
	{ 89,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"[[",
			"[[",
			"] ",
			"] ",
			"] ",
			"  ",
			"  " } },

	// letter N. 90 " Z "
	{ 90,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"#[",
			" [",
			"] ",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 91 " [ "
	{ 91,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"# ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"# ",
			"  " } },

	// letter N. 92 " \ "
	{ 92,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[ ",
			"[ ",
			"] ",
			"] ",
			"] ",
			" [",
			" [",
			"  " } },

	// letter N. 93 " ] "
	{ 93,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"# ",
			"] ",
			"] ",
			"] ",
			"] ",
			"] ",
			"# ",
			"  " } },

	// letter N. 94 " ^ "
	{ 94,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"] ",
			"[[",
			"  ",
			"  ",
			"  ",
			"  ",
			"  " } },

	// letter N. 95 "   "
	{ 95,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"  ",
			"  ",
			"  ",
			"#[",
			"  ",
			"  " } },

	// letter N. 96 " ` "
	{ 96,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[ ",
			"] ",
			" [",
			"  ",
			"  ",
			"  ",
			"  ",
			"  " } },

	// letter N. 97 " a "
	{ 97,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			" [",
			"#[",
			"#[",
			"  ",
			"  " } },

	// letter N. 98 " b "
	{ 98,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[ ",
			"[ ",
			"#[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 99 " c "
	{ 99,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[ ",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 100 " d "
	{ 100,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	" [",
			" [",
			"#[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 101 " e "
	{ 101,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"#[",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 102 " f "
	{ 102,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"][",
			"] ",
			"#[",
			"] ",
			"] ",
			"] ",
			"  ",
			"  " } },

	// letter N. 103 " g "
	{ 103,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			" [",
			"#[" } },

	// letter N. 104 " h "
	{ 104,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[ ",
			"[ ",
			"#[",
			"[[",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 105 " i "
	{ 105,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[",
			" ",
			"[",
			"[",
			"[",
			"[",
			" ",
			" " } },

	// letter N. 106 " j "
	{ 106,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"] ",
			"  ",
			"] ",
			"] ",
			"] ",
			"] ",
			"] ",
			"# " } },

	// letter N. 107 " k "
	{ 107,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[ ",
			"[ ",
			"[[",
			"# ",
			"# ",
			"[[",
			"  ",
			"  " } },

	// letter N. 108 " l "
	{ 108,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{	"[ ",
			"[ ",
			"[ ",
			"[ ",
			"[ ",
			"# ",
			"  ",
			"  " } },

	// letter N. 109 " m "
	{ 109,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"   ",
			"##[",
			"[ [",
			"[ [",
			"[ [",
			"   ",
			"   " } },

	// letter N. 110 " n "
	{ 110,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[[",
			"[[",
			"[[",
			"  ",
			"  " } },

	// letter N. 111 " o "
	{ 111,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 112 " p "
	{ 112,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			"[ ",
			"[ " } },

	// letter N. 113 " q "
	{ 113,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[[",
			"[[",
			"#[",
			" [",
			" [" } },

	// letter N. 114 " r "
	{ 114,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[ ",
			"[ ",
			"[ ",
			"  ",
			"  " } },

	// letter N. 115 " s "
	{ 115,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			"[ ",
			" [",
			"#[",
			"  ",
			"  " } },

	// letter N. 116 " t "
	{ 116,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"] ",
			"#[",
			"] ",
			"] ",
			"][",
			"  ",
			"  " } },

	// letter N. 117 " u "
	{ 117,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"[[",
			"[[",
			"[[",
			"#[",
			"  ",
			"  " } },

	// letter N. 118 " v "
	{ 118,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"[[",
			"[[",
			"[[",
			"] ",
			"  ",
			"  " } },

	// letter N. 119 " w "
	{ 119,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"   ",
			"[ [",
			"[ [",
			"[ [",
			"]] ",
			"   ",
			"   " } },

	// letter N. 120 " x "
	{ 120,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"[[",
			"] ",
			"] ",
			"[[",
			"  ",
			"  " } },

	// letter N. 121 " y "
	{ 121,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"[[",
			"[[",
			"[[",
			"#[",
			" [",
			"#[" } },

	// letter N. 122 " z "
	{ 122,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"  ",
			"  ",
			"#[",
			" [",
			"[ ",
			"#[",
			"  ",
			"  " } },

	// letter N. 123 " { "
	{ 123,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"][",
			"] ",
			"] ",
			"[ ",
			"] ",
			"] ",
			"][",
			"  " } },

	// letter N. 124 " | "
	{ 124,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"[",
			"[",
			"[",
			"[",
			"[",
			"[",
			"[",
			" " } },

	// letter N. 125 " } "
	{ 125,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"][",
			"] ",
			"] ",
			"[ ",
			"] ",
			"] ",
			"][",
			"  " } },

	// letter N. 126 " ~ "
	{ 126,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{	"   ",
			"   ",
			"   ",
			"[# ",
			"][[",
			"   ",
			"   ",
			"  " } }
};

// clang-format on

static unsigned const FontSize = sizeof(characters) / sizeof(characters[0]);
Banner maxii(characters, Hardblank, FontHeight, FontMaxLen, FontSize, FIGLET_FULLWIDTH, "*#[]", "\xdb\xdb\xdd\xde");

} // namespace Figlet
