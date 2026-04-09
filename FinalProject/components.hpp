struct CircleComponent
{
	float radius;	
};

struct SquareComponent
{
	float half_size;		// half of the length of each side
};

struct PositionComponent
{
	Vector2 position;		// center
};

struct ColorComponent
{
	Color color;
};

struct SpriteComponent
{
	Texture sprite_sheet;
	std::vector<Rectangle> frames;
	int frame_number;
};

struct MoveComponent
{
	Vector2 velocity;
};

struct AccelerationComponent
{
	Vector2 acceleration;
};

struct PhysicsComponent
{
	float mass;
	float inverse_mass;
};

struct DirectionComponent
{
	Vector2 forward;		// forward vector
};

struct InteractableComponent
{
	bool isEnabled;			// can only be interacted with if enabled

	bool isHot;				// if player presses key to interact and this is hot,
							// player will interact with this object
};

struct InteractorComponent
{
	// referencing entities
	// source: https://github.com/skypjack/entt/discussions/621

	entt::entity hot_item;	// item that this can interact with at the moment
};

struct ChairComponent
{
	entt::entity customer;
};

struct TableComponent
{
	bool hasItemOnTop;
};

struct DiningTableComponent
{
	entt::entity chair1;
	// entt::entity chair2;
};

// can be placed on a table
struct PlaceableComponent
{
	entt::entity table;		// entity it is placed on
};

struct HoldableComponent
{
	bool isHeld;
};

struct HolderComponent
{
	entt::entity held_item; // item that this is currently holding
};

struct DrinkComponent
{
	std::string name;
};

struct IngredientComponent
{
	std::string name;
	bool isPitcher;			// for water and milk pitchers
};

struct StackComponent
{
	std::string type; 		// cup or ingredient
};

struct CoffeeMachineComponent
{
	bool hasCoffeeGrounds;
	bool hasWater;
	entt::entity drink;		// empty cup or actual drink
};

struct TimerComponent
{
	float time;
};

struct CustomerComponent
{
	float patience;
	std::string state;
	std::string order;
	entt::entity table;
	entt::entity drink;
};

struct MoneyComponent
{
	float amount;
};