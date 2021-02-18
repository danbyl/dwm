static const DmenuArg actionarg = {
	.items = (DmenuItem[]){
		{
			.name = "toggle settings",
			.arg = {
				.v = &(DmenuArg){
					.items = (DmenuItem[]){
						{ .name = "gaps",       .func = togglegaps },
						{ .name = "borders",    .func = toggleborders },
						{ .name = "bar",        .func = togglebar },
						{ .name = "swallowing", .func = toggleswallow },
						{ 0 }
					},
					.prompt = "Toggle:",
				}
			}
		}, {
			.name = "change layout",
			.arg = {
				.v = &(DmenuArg){
					.items = (DmenuItem[]){
						{ .name = "tiling",                   .arg = {.v = &layouts[0] } },
						{ .name = "bottom stack",             .arg = {.v = &layouts[1] } },
						{ .name = "monocle",                  .arg = {.v = &layouts[2] } },
						{ .name = "deck",                     .arg = {.v = &layouts[3] } },
						{ .name = "centered master",          .arg = {.v = &layouts[4] } },
						{ .name = "centered floating master", .arg = {.v = &layouts[5] } },
						{ .name = "floating",                 .arg = {.v = &layouts[6] } },
						{ 0 }
					},
					.prompt = "Layout:",
					.func = setlayout,
				}
			}
		}, {
			.name = "change attach function",
			.arg = {
				.v = &(DmenuArg){
					.items = (DmenuItem[]){
						{ .name = "as master",                 .arg = {.ui = AttachMaster } },
						{ .name = "above the selected client", .arg = {.ui = AttachAbove } },
						{ .name = "below the selected client", .arg = {.ui = AttachBelow } },
						{ .name = "on the top of the stack",   .arg = {.ui = AttachTop } },
						{ .name = "on the bottom of stack",    .arg = {.ui = AttachBottom } },
						{ 0 }
					},
					.prompt = "Attach:",
					.func = setattach,
				}
			}
		}, {
			.name = "reload xresources", .func = xrdb
		}, {
			.name = "set master factor",
			.arg = {
				.v = &(DmenuArg){
					.items = (DmenuItem[]){{0}},
					.prompt = "Set master factor to:",
					.func = strsetmfact,
					.strarg = 1,
				}
			}
		}, { 0 }
	},
	.prompt = "What to do?",
	.func = dmenuaction,
};
