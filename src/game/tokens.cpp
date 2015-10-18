#include "tokens.hpp"
#include <fstream>
#include "Utilities.h"
#include "../json/json.h"
extern "C" {
#include "g_local.h"
}

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif


Tokens::Tokens()
{
}


Tokens::~Tokens()
{
}


std::pair<bool, std::string> Tokens::createToken(Difficulty difficulty, std::array<float, 3> coordinates)
{
	std::array<Token, TOKENS_PER_DIFFICULTY> *tokens = nullptr;
	switch (difficulty)
	{
	case Easy:
		tokens = &_easyTokens;
		break;
	case Medium:
		tokens = &_mediumTokens;
		break;
	case Hard:
		tokens = &_hardTokens;
		break;
	}

	Token *nextFreeToken = nullptr;
	auto idx = 0;
	for (auto & token : *tokens)
	{
		if (!token.isActive)
		{
			nextFreeToken = &token;
			break;
		}
		++idx;
	}

	if (nextFreeToken == nullptr)
	{
		return std::make_pair(false, "no free tokens left for the difficulty.");
	}

	nextFreeToken->isActive = true;
	nextFreeToken->name = "";
	nextFreeToken->coordinates = coordinates;
	nextFreeToken->data->idx = idx;

	createEntity(*nextFreeToken, difficulty);

	if (!saveTokens(_filepath))
	{
		return std::make_pair(false, "Could not save tokens to a file. Check logs for more information.");
	}

	return std::make_pair(true, "");
}

bool Tokens::loadTokens(const std::string& filepath)
{
	_filepath = filepath;
	std::string content;
	try
	{
		content = Utilities::ReadFile(filepath);
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not read file: ") + e.what());
		return false;
	}
	
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(content, root))
	{
		Utilities::Logln("Tokens: Could not parse file \"" + filepath + "\".");
		Utilities::Logln("Tokens: " + reader.getFormattedErrorMessages());
		return false;
	}

	try
	{
		auto easyTokens = root["easyTokens"];
		auto mediumTokens = root["mediumTokens"];
		auto hardTokens = root["hardTokens"];

		auto idx = 0;
		for (auto&easyToken:easyTokens)
		{
			_easyTokens[idx].fromJson(easyToken);
			_easyTokens[idx].data->idx = idx;
			++idx;
		}

		idx = 0;
		for (auto&mediumToken:mediumTokens)
		{
			_mediumTokens[idx].fromJson(mediumToken);
			_mediumTokens[idx].data->idx = idx;
			++idx;
		}

		idx = 0;
		for (auto&hardToken:hardTokens)
		{
			_hardTokens[idx].fromJson(hardToken);
			_hardTokens[idx].data->idx = idx;
			++idx;
		}
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not parse configuration from file \"" + filepath + "\": ") + e.what());
		return false;
	}

	createEntities();

	Utilities::Logln("Tokens: Successfully loaded all tokens from \"" + filepath + "\" for current map.");

	return false;
}


bool allTokensCollected(gentity_t *ent);
void Tokens::createEntity(Token& token, Difficulty difficulty)
{
	token.entity = G_Spawn();
	token.entity->tokenInformation = token.data.get();
	Q_strncpyz(token.data->name, token.name.c_str(), sizeof(token.data->name));
	token.data->difficulty = difficulty;
	
	switch (difficulty)
	{
	case Easy:
		token.entity->classname = "token_easy";
		break;
	case Medium:
		token.entity->classname = "token_medium";
		break;
	case Hard:
		token.entity->classname = "token_hard";
		break;
	}

	token.entity->think = [](gentity_t *self)
	{
		vec3_t mins = { 0, 0, 0 };
		vec3_t maxs = { 0, 0, 0 };
		vec3_t range = { 16, 16, 16 };
		VectorSubtract(self->r.currentOrigin, range, mins);
		VectorAdd(self->r.currentOrigin, range, maxs);

		int entityList[MAX_GENTITIES];
		auto count = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);
		for (auto i = 0; i < count; ++i)
		{
			auto ent = &g_entities[entityList[i]];

			if (!ent->client)
			{
				continue;
			}

			const char *difficulty = NULL;
			qboolean *collected = NULL;
			switch (self->tokenInformation->difficulty)
			{
			case Easy:
				difficulty = "^2easy";
				collected = &ent->client->pers.collectedEasyTokens[self->tokenInformation->idx];
				break;
			case Medium:
				difficulty = "^3medium";
				collected = &ent->client->pers.collectedMediumTokens[self->tokenInformation->idx];
				break;
			case Hard:
				difficulty = "^1hard";
				collected = &ent->client->pers.collectedHardTokens[self->tokenInformation->idx];
				break;
			default:
				G_Error("Visited unknown difficulty.");
			}

			if (*collected)
			{
				continue;
			}

			*collected = qtrue;
			C_CPMTo(ent, va("^7You collected %s ^7token ^5#%d", difficulty, self->tokenInformation->idx + 1));

			if (allTokensCollected(ent))
			{
				auto millis = level.time - ent->client->pers.tokenCollectionStartTime;
				int minutes, seconds;
				minutes = millis / 60000;
				millis -= minutes * 60000;
				seconds = millis / 1000;
				millis -= seconds * 1000;

				C_CPMAll(va("%s ^7collected all tokens in %02d:%02d:%03d", ent->client->pers.netname, minutes, seconds, millis));
			}
		}

		self->nextthink = level.time + FRAMETIME;
	};

	token.entity->nextthink = level.time + FRAMETIME;
	G_SetOrigin(token.entity, token.coordinates.data());
	trap_LinkEntity(token.entity);
}


std::array<int, 3> Tokens::getTokenCounts() const
{
	std::array<int, 3> ret;
	for (auto&val:ret)
	{
		val = 0;
	}
	for (auto&t : _easyTokens)
	{
		if (t.isActive)
		{
			++ret[0];
		}
	}
	for (auto&t : _mediumTokens)
	{
		if (t.isActive)
		{
			++ret[1];
		}
	}
	for (auto&t : _hardTokens)
	{
		if (t.isActive)
		{
			++ret[2];
		}
	}
	return ret;
}

void Tokens::reset()
{
	for (auto&t : _easyTokens)
	{
		t.isActive = false;
	}
	for (auto&t : _mediumTokens)
	{
		t.isActive = false;
	}
	for (auto&t : _hardTokens)
	{
		t.isActive = false;
	}
}

void Tokens::createEntities()
{
	for (auto&t : _easyTokens)
	{
		if (t.isActive)
		{
			createEntity(t, Easy);
		}
	}
	for (auto&t : _mediumTokens)
	{
		if (t.isActive)
		{
			createEntity(t, Medium);
		}
	}
	for (auto&t : _hardTokens)
	{
		if (t.isActive)
		{
			createEntity(t, Hard);
		}
	}
}


bool Tokens::saveTokens(const std::string& filepath)
{
	Json::Value root;
	root["easyTokens"] = Json::arrayValue;
	root["mediumTokens"] = Json::arrayValue;
	root["hardTokens"] = Json::arrayValue;
	for (auto&token:_easyTokens)
	{
		if (token.isActive)
		{
			root["easyTokens"].append(token.toJson());
		}
	}

	for (auto&token:_mediumTokens)
	{
		if (token.isActive)
		{
			root["mediumTokens"].append(token.toJson());
		}
	}

	for (auto&token:_hardTokens)
	{
		if (token.isActive)
		{
			root["hardTokens"].append(token.toJson());
		}
	}

	Json::StyledWriter writer;
	auto output = writer.write(root);
	try
	{
		Utilities::WriteFile(filepath, output);
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not save tokens to file: ") + e.what());
		return false;
	}

	Utilities::Logln("Tokens: Saved all tokens to \"" + filepath + "\"");

	return true;
}

void Tokens::Token::fromJson(const Json::Value& json)
{	
	if (json["coordinates"].size() != 3)
	{
		throw std::runtime_error("Coordinates array should have 3 items.");
	}
	coordinates[0] = json["coordinates"][0].asFloat();
	coordinates[1] = json["coordinates"][1].asFloat();
	coordinates[2] = json["coordinates"][2].asFloat();
	name = json["name"].asString();
	isActive = true;
}

Tokens::Token::Token() : coordinates{ 0,0,0 }, name(""), isActive(false), entity(nullptr), data(std::make_unique<TokenInformation>())
{
}

Json::Value Tokens::Token::toJson() const
{
	Json::Value jsonToken;
	jsonToken["coordinates"] = Json::arrayValue;
	jsonToken["coordinates"].append(coordinates[0]);
	jsonToken["coordinates"].append(coordinates[1]);
	jsonToken["coordinates"].append(coordinates[2]);

	jsonToken["name"] = name;

	return jsonToken;
}