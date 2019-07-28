// JP: maybe fix one day
#if 0
struct TradeableBoostFixture {
	TradeableBoostFixture() : boost() {
		g_DetailRecipeDict.itemsFromEnhancementName = stashTableCreateWithStringKeys(2, StashDeepCopyKeys);
		sPowerPath = "set.boost";
	}
	~TradeableBoostFixture() {
		stashTableDestroy(g_DetailRecipeDict.itemsFromEnhancementName);
	}
	BasePower boost;
	DetailRecipe recipe;
};

TEST_FIXTURE(TradeableBoostFixture, uncraftedDetailRecipeTradeable) {
	boost.bBoostTradeable = false;
	sIsSetCrafted = false;
	CHECK(!detailrecipedict_IsBoostTradeable(NULL, 0, "", ""));
	CHECK(!detailrecipedict_IsBoostTradeable(&boost, 0, "", ""));
	boost.bBoostTradeable = true;
	CHECK(detailrecipedict_IsBoostTradeable(&boost, 0, "", ""));
}

TEST_FIXTURE(TradeableBoostFixture, boostsFromTradeableRecipesTradeable) {
	boost.bBoostTradeable = true;
	sIsSetCrafted = true;
	recipe.flags = 0;
	stashAddPointer(g_DetailRecipeDict.itemsFromEnhancementName, "set.boost_11", 
		(DetailRecipe*)&recipe, false);
	CHECK(detailrecipedict_IsBoostTradeable(&boost, 10, "", ""));
	CHECK(!detailrecipedict_IsBoostTradeable(&boost, 5, "", ""));
}

TEST_FIXTURE(TradeableBoostFixture, boostsFromUntradeableRecipesUntradeable) {
	boost.bBoostTradeable = true;
	sIsSetCrafted = true;
	recipe.flags = RECIPE_NO_TRADE;
	stashAddPointer(g_DetailRecipeDict.itemsFromEnhancementName, "set.boost_11",
		(DetailRecipe*)&recipe, false);
	CHECK(!detailrecipedict_IsBoostTradeable(&boost, 10, "", ""));
}
#endif
