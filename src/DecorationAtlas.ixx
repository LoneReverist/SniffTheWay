// DecorationAtlas.ixx

module;

#include <array>
#include <cstdint>

export module DecorationAtlas;

export namespace SniffTheWay::Decorations
{
	constexpr std::uint32_t TextureWidth = 1536;
	constexpr std::uint32_t TextureHeight = 1024;
	constexpr char const * TextureFileName = "decorations.png";

	struct PixelBounds
	{
		std::uint32_t x = 0;
		std::uint32_t y = 0;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};

	enum class DecorationCategory : std::uint8_t
	{
		HorizontalDivider,
		OrnateDivider,
		ShortDivider,
		Accent,
		CornerAccent,
		SectionBreak,
		PawPrint,
		TextOrnament,
		Icon,
	};

	enum class DecorationId : std::uint8_t
	{
		HorizontalDividerPawArrows,
		HorizontalDividerPawLines,
		HorizontalDividerLeafPawArrows,
		HorizontalDividerPawDots,
		HorizontalDividerPawFlourish,
		HorizontalDividerLeafFlourishPaw,
		OrnateDividerLoopDiamond,
		OrnateDividerPawVines,
		OrnateDividerCenterDiamond,
		OrnateDividerInterwoven,
		OrnateDividerLeafCenter,
		ShortDividerDiamonds,
		ShortDividerPawArrows,
		ShortDividerTripleDiamonds,
		ShortDividerLeafPaw,
		AccentVerticalDotsDiamonds,
		AccentVerticalStars,
		AccentVerticalLeaf,
		AccentStarTop,
		AccentStarMiddle,
		AccentStarBottom,
		CornerFloralLarge,
		CornerFloralMedium,
		CornerFloralSmall,
		CornerFloralTiny,
		SectionBreakLeaf,
		SectionBreakSun,
		SectionBreakDiamond,
		SectionBreakFlower,
		PawSolid,
		PawOutline,
		PawGlow,
		PawSpeckled,
		TextOrnamentLeafLeft,
		TextOrnamentStarSmall,
		TextOrnamentStarLarge,
		TextOrnamentDot,
		TextOrnamentLoop,
		TextOrnamentLeafRight,
		TextOrnamentPawLine,
		IconButterfly,
		IconLeafSprig,
		IconFlower,
		IconMushroom,
		IconSpark,
		IconFlowerRound,
		IconFeather,
		IconHeart,
		IconLeaf,
		IconCrescent,
	};

	struct DecorationInfo
	{
		DecorationId id;
		char const * name;
		DecorationCategory category;
		PixelBounds bounds;
	};

	constexpr std::array<DecorationInfo, 50> AllDecorations{
		DecorationInfo{ DecorationId::HorizontalDividerPawArrows, "horizontal_divider_paw_arrows",
			DecorationCategory::HorizontalDivider, { 65, 121, 375, 39 } },
		DecorationInfo{ DecorationId::HorizontalDividerPawLines, "horizontal_divider_paw_lines",
			DecorationCategory::HorizontalDivider, { 65, 196, 375, 39 } },
		DecorationInfo{ DecorationId::HorizontalDividerLeafPawArrows, "horizontal_divider_leaf_paw_arrows",
			DecorationCategory::HorizontalDivider, { 65, 271, 375, 41 } },
		DecorationInfo{ DecorationId::HorizontalDividerPawDots, "horizontal_divider_paw_dots",
			DecorationCategory::HorizontalDivider, { 78, 357, 350, 34 } },
		DecorationInfo{ DecorationId::HorizontalDividerPawFlourish, "horizontal_divider_paw_flourish",
			DecorationCategory::HorizontalDivider, { 45, 436, 414, 42 } },
		DecorationInfo{ DecorationId::HorizontalDividerLeafFlourishPaw, "horizontal_divider_leaf_flourish_paw",
			DecorationCategory::HorizontalDivider, { 47, 520, 411, 49 } },

		DecorationInfo{ DecorationId::OrnateDividerLoopDiamond, "ornate_divider_loop_diamond",
			DecorationCategory::OrnateDivider, { 544, 125, 387, 41 } },
		DecorationInfo{ DecorationId::OrnateDividerPawVines, "ornate_divider_paw_vines",
			DecorationCategory::OrnateDivider, { 545, 214, 386, 43 } },
		DecorationInfo{ DecorationId::OrnateDividerCenterDiamond, "ornate_divider_center_diamond",
			DecorationCategory::OrnateDivider, { 545, 304, 386, 46 } },
		DecorationInfo{ DecorationId::OrnateDividerInterwoven, "ornate_divider_interwoven",
			DecorationCategory::OrnateDivider, { 546, 389, 383, 67 } },
		DecorationInfo{ DecorationId::OrnateDividerLeafCenter, "ornate_divider_leaf_center",
			DecorationCategory::OrnateDivider, { 539, 497, 397, 62 } },

		DecorationInfo{ DecorationId::ShortDividerDiamonds, "short_divider_diamonds",
			DecorationCategory::ShortDivider, { 1019, 136, 191, 19 } },
		DecorationInfo{ DecorationId::ShortDividerPawArrows, "short_divider_paw_arrows",
			DecorationCategory::ShortDivider, { 1019, 195, 191, 39 } },
		DecorationInfo{ DecorationId::ShortDividerTripleDiamonds, "short_divider_triple_diamonds",
			DecorationCategory::ShortDivider, { 1019, 275, 191, 25 } },
		DecorationInfo{ DecorationId::ShortDividerLeafPaw, "short_divider_leaf_paw",
			DecorationCategory::ShortDivider, { 1011, 342, 207, 35 } },

		DecorationInfo{ DecorationId::AccentVerticalDotsDiamonds, "accent_vertical_dots_diamonds",
			DecorationCategory::Accent, { 1274, 135, 13, 242 } },
		DecorationInfo{ DecorationId::AccentVerticalStars, "accent_vertical_stars", DecorationCategory::Accent,
			{ 1348, 137, 19, 91 } },
		DecorationInfo{ DecorationId::AccentVerticalLeaf, "accent_vertical_leaf", DecorationCategory::Accent,
			{ 1346, 242, 23, 135 } },
		DecorationInfo{ DecorationId::AccentStarTop, "accent_star_top", DecorationCategory::Accent,
			{ 1429, 164, 28, 34 } },
		DecorationInfo{ DecorationId::AccentStarMiddle, "accent_star_middle", DecorationCategory::Accent,
			{ 1428, 241, 30, 35 } },
		DecorationInfo{ DecorationId::AccentStarBottom, "accent_star_bottom", DecorationCategory::Accent,
			{ 1426, 315, 33, 36 } },

		DecorationInfo{ DecorationId::CornerFloralLarge, "corner_floral_large", DecorationCategory::CornerAccent,
			{ 1006, 490, 86, 85 } },
		DecorationInfo{ DecorationId::CornerFloralMedium, "corner_floral_medium", DecorationCategory::CornerAccent,
			{ 1138, 491, 82, 83 } },
		DecorationInfo{ DecorationId::CornerFloralSmall, "corner_floral_small", DecorationCategory::CornerAccent,
			{ 1267, 492, 79, 82 } },
		DecorationInfo{ DecorationId::CornerFloralTiny, "corner_floral_tiny", DecorationCategory::CornerAccent,
			{ 1387, 488, 83, 87 } },

		DecorationInfo{ DecorationId::SectionBreakLeaf, "section_break_leaf", DecorationCategory::SectionBreak,
			{ 45, 685, 415, 35 } },
		DecorationInfo{ DecorationId::SectionBreakSun, "section_break_sun", DecorationCategory::SectionBreak,
			{ 44, 748, 416, 43 } },
		DecorationInfo{ DecorationId::SectionBreakDiamond, "section_break_diamond", DecorationCategory::SectionBreak,
			{ 45, 835, 415, 40 } },
		DecorationInfo{ DecorationId::SectionBreakFlower, "section_break_flower", DecorationCategory::SectionBreak,
			{ 44, 908, 417, 40 } },

		DecorationInfo{ DecorationId::PawSolid, "paw_solid", DecorationCategory::PawPrint, { 536, 696, 62, 64 } },
		DecorationInfo{ DecorationId::PawOutline, "paw_outline", DecorationCategory::PawPrint, { 650, 696, 64, 64 } },
		DecorationInfo{ DecorationId::PawGlow, "paw_glow", DecorationCategory::PawPrint, { 754, 690, 90, 75 } },
		DecorationInfo{ DecorationId::PawSpeckled, "paw_speckled", DecorationCategory::PawPrint,
			{ 883, 689, 67, 76 } },

		DecorationInfo{ DecorationId::TextOrnamentLeafLeft, "text_ornament_leaf_left",
			DecorationCategory::TextOrnament, { 542, 875, 41, 30 } },
		DecorationInfo{ DecorationId::TextOrnamentStarSmall, "text_ornament_star_small",
			DecorationCategory::TextOrnament, { 615, 877, 25, 28 } },
		DecorationInfo{ DecorationId::TextOrnamentStarLarge, "text_ornament_star_large",
			DecorationCategory::TextOrnament, { 679, 878, 26, 27 } },
		DecorationInfo{ DecorationId::TextOrnamentDot, "text_ornament_dot", DecorationCategory::TextOrnament,
			{ 746, 886, 12, 13 } },
		DecorationInfo{ DecorationId::TextOrnamentLoop, "text_ornament_loop", DecorationCategory::TextOrnament,
			{ 795, 884, 60, 17 } },
		DecorationInfo{ DecorationId::TextOrnamentLeafRight, "text_ornament_leaf_right",
			DecorationCategory::TextOrnament, { 887, 874, 68, 31 } },
		DecorationInfo{ DecorationId::TextOrnamentPawLine, "text_ornament_paw_line",
			DecorationCategory::TextOrnament, { 540, 954, 485, 33 } },

		DecorationInfo{ DecorationId::IconButterfly, "icon_butterfly", DecorationCategory::Icon,
			{ 1042, 707, 58, 52 } },
		DecorationInfo{ DecorationId::IconLeafSprig, "icon_leaf_sprig", DecorationCategory::Icon,
			{ 1141, 707, 43, 53 } },
		DecorationInfo{ DecorationId::IconFlower, "icon_flower", DecorationCategory::Icon, { 1234, 706, 40, 54 } },
		DecorationInfo{ DecorationId::IconMushroom, "icon_mushroom", DecorationCategory::Icon,
			{ 1328, 710, 37, 49 } },
		DecorationInfo{ DecorationId::IconSpark, "icon_spark", DecorationCategory::Icon, { 1419, 706, 41, 54 } },
		DecorationInfo{ DecorationId::IconFlowerRound, "icon_flower_round", DecorationCategory::Icon,
			{ 1047, 834, 43, 49 } },
		DecorationInfo{ DecorationId::IconFeather, "icon_feather", DecorationCategory::Icon,
			{ 1138, 825, 44, 63 } },
		DecorationInfo{ DecorationId::IconHeart, "icon_heart", DecorationCategory::Icon, { 1231, 839, 44, 41 } },
		DecorationInfo{ DecorationId::IconLeaf, "icon_leaf", DecorationCategory::Icon, { 1325, 834, 44, 50 } },
		DecorationInfo{ DecorationId::IconCrescent, "icon_crescent", DecorationCategory::Icon,
			{ 1421, 835, 42, 48 } },
	};

	constexpr DecorationInfo const & Get(DecorationId id)
	{
		return AllDecorations[static_cast<std::uint8_t>(id)];
	}
}
