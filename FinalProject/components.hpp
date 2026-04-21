#include "json.hpp"
#include "entt.hpp"

const float GRID_SIZE = 48.0f;
const float radius = 16.0f;

float brew_time = 10.0f;

float score = 0;
float day_score = 0;

entt::registry registry;
std::vector<entt::entity> available_tables;

// TEXTURES
Texture bean;
Texture hot_coffee;
Texture coffee_tools;
Texture iced_coffee;
Texture user;
Texture kitchen;
Texture water;
Texture espresso;
Texture americano;
Texture cappuccino;
Texture order;
Texture recipes;

Texture textures[12] = {
    bean, hot_coffee, coffee_tools, iced_coffee, user, kitchen,
    water, espresso, americano, cappuccino, order, recipes
};

struct CircleComponent
{
	float radius;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CircleComponent, radius)
};

struct SquareComponent
{
	float half_size;		// half of the length of each side

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SquareComponent, half_size)
};

struct PositionComponent
{
	Vector2 position;		// center

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PositionComponent, position)
};

inline void to_json(nlohmann::json& j, const Vector2& v) {
    j = {{"x", v.x}, {"y", v.y}};
}

inline void from_json(const nlohmann::json& j, Vector2& v) {
    v.x = j.at("x").get<float>();
    v.y = j.at("y").get<float>();
}

struct ColorComponent
{
	Color color;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorComponent, color)
};

inline void to_json(nlohmann::json& j, const ColorComponent& c) {
    j = {{"r", c.color.r}, {"g", c.color.g}, {"b", c.color.b}, {"a", c.color.a}};
}

inline void from_json(const nlohmann::json& j, ColorComponent& c) {
    j.at("r").get_to(c.color.r);
    j.at("g").get_to(c.color.g);
    j.at("b").get_to(c.color.b);
    j.at("a").get_to(c.color.a);
}


struct SpriteComponent
{
	int sprite_id;
	//Texture sprite_sheet;
	std::vector<Rectangle> frames;
	int frame_number;
	Vector2 origin;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(SpriteComponent, sprite_id, frames, frame_number, origin)
};

inline void to_json(nlohmann::json& j, const Rectangle& r) {
    j = {
        {"x", r.x},
        {"y", r.y},
        {"width", r.width},
        {"height", r.height}
    };
}

inline void from_json(const nlohmann::json& j, Rectangle& r) {
    r.x = j.at("x").get<float>();
    r.y = j.at("y").get<float>();
    r.width = j.at("width").get<float>();
    r.height = j.at("height").get<float>();
}

struct MoveComponent
{
	Vector2 velocity;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MoveComponent, velocity)
};

struct AccelerationComponent
{
	Vector2 acceleration;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AccelerationComponent, acceleration)
};

struct PhysicsComponent
{
	float mass;
	float inverse_mass;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicsComponent, mass, inverse_mass)
};

struct DirectionComponent
{
	Vector2 forward;		// forward vector

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DirectionComponent, forward)
};

struct InteractableComponent
{
	bool isEnabled;			// can only be interacted with if enabled

	bool isHot;				// if player presses key to interact and this is hot,
							// player will interact with this object

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(InteractableComponent, isEnabled, isHot)
};

struct InteractorComponent
{
	// referencing entities
	// source: https://github.com/skypjack/entt/discussions/621

	entt::entity hot_item;	// item that this can interact with at the moment

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(InteractorComponent, hot_item)
};

struct ChairComponent
{
	entt::entity customer;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ChairComponent, customer)
};

struct TableComponent
{
	bool hasItemOnTop;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TableComponent, hasItemOnTop)
};

struct DiningTableComponent
{
	entt::entity chair1;
	// entt::entity chair2;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DiningTableComponent, chair1)
};

// can be placed on a table
struct PlaceableComponent
{
	entt::entity table;		// entity it is placed on

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlaceableComponent, table)
};

struct HoldableComponent
{
	bool isHeld;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(HoldableComponent, isHeld)
};

struct HolderComponent
{
	entt::entity held_item; // item that this is currently holding

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(HolderComponent, held_item)
};

struct DrinkComponent
{
	std::string name;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(DrinkComponent, name)
};

struct IngredientComponent
{
	std::string name;
	bool isPitcher;			// for water and milk pitchers

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(IngredientComponent, name, isPitcher)
};

struct StackComponent
{
	std::string type; 		// cup or ingredient

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(StackComponent, type)
};

struct CoffeeMachineComponent
{
	bool hasCoffeeGrounds;
	bool hasWater;
	entt::entity drink;		// empty cup or actual drink

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CoffeeMachineComponent, hasCoffeeGrounds, hasWater, drink)
};

struct TimerComponent
{
	float time;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimerComponent, time)
};

struct CustomerComponent
{
	float patience;
	std::string order;
	entt::entity table;
	entt::entity drink;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CustomerComponent, patience, state, order, table, drink)
};

struct MoneyComponent
{
	float amount;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MoneyComponent, amount)
};