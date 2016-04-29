#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "utils.h"
#include "beatmap.h"
#include "preview_window.h"
#include "diff_calc.h"
#include "pp_calc.h"

namespace {
	beatmap b;
#ifdef SHOW_BEATMAP
	void print_beatmap();
#endif
}

namespace {
	const char* const mod_strs[] {
		"nomod", "nf", "ez", "hd", "hr", "dt", "ht", "nc", "fl", "so"
	};
	const u32 mod_masks[] {
		mods::nomod, mods::nf, mods::ez, mods::hd, mods::hr, mods::dt, mods::ht,
		mods::nc, mods::fl, mods::so
	};
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s /path/to/difficulty.osu [acc]%% +[mods] "
				"[combo]x [misses]xmiss\n\n", *argv);
		puts("acc: the accuracy in percent (example: 99.99%)");
		puts("mods (optional): any combination of nomod, nf, ez, hd, hr, dt, ht"
				", nc, fl, so (example: +hddthr)");
		puts("combo: the highest combo (example: 1337x)");
		puts("misses: amount of misses (example: 1xmiss)");
		puts("(the order of the optional arguments does not matter)");

		return 0;
	}

	beatmap::parse(argv[1], b);

	char* mods_str = nullptr;
	f64 acc = 100.0;
	u32 mods = mods::nomod;
	u16 combo = b.max_combo;
	u16 misses = 0;

	for (int i = 2; i < argc; i++) {
		char suff[64] = {0};
		auto a = argv[i];

		std::transform(a, a + strlen(a), a, tolower);

		// acc
		f64 tmp_acc;
		if (sscanf(a, "%lf%s", &tmp_acc, suff) == 2 && !strcmp(suff, "%")) {
			acc = tmp_acc;
			continue;
		}

		// mods
		for (size_t j = 0; j < sizeof(mod_masks) / sizeof(mod_masks[0]); j++) {
			if (strstr(a, mod_strs[j])) {
				mods_str = a;
				mods |= mod_masks[j];
			}
		}

		if (mods_str == a) {
			if (*mods_str == '+') {
				std::transform(mods_str, mods_str + strlen(mods_str), mods_str, 
						toupper);
				continue;
			} else {
				mods_str = nullptr;
			}
		}

		// combo
		u16 tmp_combo;
		if (sscanf(a, "%hd%s", &tmp_combo, suff) == 2 && !strcmp(suff, "x")) {
			combo = tmp_combo;
			continue;
		}

		// misses
		u16 tmp_misses;
		if (sscanf(a, "%hd%s", &tmp_misses, suff) == 2 && 
				!strcmp(suff, "xmiss")) {
			misses = tmp_misses;
			continue;
		}

		printf(">%s\n", a);
		die("Invalid parameter");
	}

#ifdef SHOW_BEATMAP
	puts("\n---");
	puts(argv[1]);
	puts("---\n");

	p_init(argc, argv);
	print_beatmap();
#endif

	printf("\n%s - %s [%s] (%s) %s\n", 
			b.artist, b.title, b.version, b.creator, mods_str ? mods_str : "");

	b.apply_mods(mods);
	printf("od%g ar%g cs%g\n", b.od, b.ar, b.cs);
	printf("%hd/%hd combo\n", combo, b.max_combo);
	printf("%hdxmiss\n", misses);

	f64 aim, speed;
	f64 stars = d_calc(b, &aim, &speed);
	printf("\n%g stars\naim stars: %g, speed stars: %g\n", stars, aim, speed);

	f64 pp = pp_calc_acc(aim, speed, b, acc, mods, combo, misses);
	printf("\n%gpp\n", pp);

	return 0;
}

#ifdef SHOW_BEATMAP
namespace {
	void print_beatmap() {
		printf(
			"Format version: %d\n"
			"Stack Leniency: %g\n"
			"Mode: %d\n"
			"Title: %s\n"
			"Artist: %s\n"
			"Version: %s\n"
			"HP%g CS%g OD%g AR%g SV%g\n\n"
			,
			b.format_version, 
			b.stack_leniency, 
			b.mode, 
			b.title, 
			b.artist, 
			b.version, 
			b.hp, b.cs, b.od, b.ar, b.sv
		);

		printf("> %ld timing points\n", b.num_timing_points);
		for (size_t i = 0; i < b.num_timing_points; i++) {
			auto& tp = b.timing_points[i];
			printf("%ld: ", tp.time);
			if (!tp.inherit) {
				printf("%g bpm\n", 60000.0 / tp.ms_per_beat);
			} else {
				printf("%gx\n", -100.0 / tp.ms_per_beat);
			}
		}

		printf("\n> %ld hit objects\n", b.num_objects);
		for (size_t i = 0; i < b.num_objects; i++) {

			auto& ho = b.objects[i];
			switch (ho.type) {
				case obj::circle:
					printf("%ld: Circle (%g, %g)\n", 
						ho.time, ho.pos.x, ho.pos.y);
					break;

				case obj::spinner:
					printf("%ld-%ld: Spinner\n", ho.time, ho.end_time);
					break;

				case obj::slider:
				{
					auto& sl = ho.slider;

					printf(
						"%ld-%ld: Slider "
						"[Type %c, Length %g, %ld Repetitions] ", 
						ho.time, ho.end_time, sl.type, 
						sl.length, sl.repetitions);

					for (size_t j = 0; j < sl.num_points; j++) {
						auto& pt = sl.points[j];
						printf("(%g, %g) ", pt.x, pt.y);
					}

					puts("");

					p_show(ho);
					break;
				}

				default:
					die("Invalid object type");
			}
		}
	}
}
#endif
