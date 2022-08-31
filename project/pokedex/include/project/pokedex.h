#define POKEDEX_NUM_POKEMON 151
#define POKEDEX_NAME_MAX    11
#define POKEDEX_ENTRY_MAX   104

typedef struct {
    const char name[POKEDEX_NAME_MAX];
    const char entry[POKEDEX_ENTRY_MAX];
} PokedexEntry;

extern const PokedexEntry pokedex[POKEDEX_NUM_POKEMON][POKEDEX_ENTRY_MAX];
