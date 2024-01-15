#include <bits/stdc++.h>

#define DEBUG
#define TIMER_START start = std::chrono::high_resolution_clock::now();
#define TIMER_END(name) end = std::chrono::high_resolution_clock::now(); std::cerr << name << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;

typedef __int128_t i128;
#define AMOUNT_OF_FILTERS 46656
#define EMPTY -1
#define BOX_OFFSET(x, y) box_offset[y][x]
#define BITMAP_OFFSET(x, y) (x + y * 9)
#define BIT(var, x, y) var & ((__int128_t)1 << (x + y * 9))
#define CREATE128(var, p1, p2) __int128_t var = ((__int128_t)p2 << 64) | (__int128_t)p1;

#define MOVE_COUNT_TO_EXTEND_THINKING 16
#define MAX_TIMEFRAME_FOR_EXTEND_THINKING (27.5 * CLOCKS_PER_SEC)

class move;
class board;
class algorithm;

void print_i128_as_grid(i128 input_num);
void load_board(board *b);
void calculate_filters();
void clean_filters(board *b, int num = 0);
void prepare_board_specefic_filters(board *b, int num = 0);

const int box_offset[9][9] = {
	{0, 0, 0, 3, 3, 3, 6, 6, 6},	
	{0, 0, 0, 3, 3, 3, 6, 6, 6},
	{0, 0, 0, 3, 3, 3, 6, 6, 6},
	{27, 27, 27, 30, 30, 30, 33, 33, 33},
	{27, 27, 27, 30, 30, 30, 33, 33, 33},
	{27, 27, 27, 30, 30, 30, 33, 33, 33},
	{54, 54, 54, 57, 57, 57, 60, 60, 60},
	{54, 54, 54, 57, 57, 57, 60, 60, 60},
	{54, 54, 54, 57, 57, 57, 60, 60, 60}
};

int current_board_state[9][9] = {
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
	{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
};

CREATE128(row_mask, 0b1000000001000000001000000001000000001000000001000000001000000001, 0b100000000)
CREATE128(col_mask, 0b111111111, 0)
CREATE128(box_mask, 0b111000000111000000111, 0)

// Filter management
i128 all_filters[9][AMOUNT_OF_FILTERS];
char filters_active[9][AMOUNT_OF_FILTERS];
int filter_count[9] = {AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS, AMOUNT_OF_FILTERS};

int current_solution[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int solve_order[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
int filter_setup_index = 0;

int amount_of_moves_played = 0;

int total_moves_calculated = 0;
int total_moves_double = 0;
int search_count = 0;

double total_time = 0;
clock_t start;

class move{
public:
	int x = -1, y = -1, num = -1;
	bool final_move = false;

	/// Nothing based
	move() {}
	/// final move
	move(bool final_move) : final_move(final_move) {}
	/// num 0 based
	move(int x, int y, int num, bool final_move = false) : x(x), y(y), num(num), final_move(final_move) {}
	/// num 1 based
	move(std::string str) : x(str[1] - 'a'), y(str[0] - 'A'), num(str[2] - '1'), final_move(str.length() == 4) {}

	std::string to_string(){
		if (final_move && x != -1) return (std::string({(char)(y + 'A'), (char)(x + 'a'), (char)(num + '1'), '!'}));
		if (final_move) return (std::string({'!'}));
		return (std::string({(char)(y + 'A'), (char)(x + 'a'), (char)(num + '1')}));
	}

	operator bool() const{
		return (x != -1);
	}

	bool operator==(const move &other) const{
		return (x == other.x && y == other.y && num == other.num);
	}
};

class board{
public:
	/// (y, x) = num
	char current_game[9][9] = {
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
	};
	char current_game_num_y_map[9][9] = {
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
		{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}
	};
	i128 digit_masks[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	i128 digit_except_masks[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	bool is_valid = true;
	int moves_played = 0;

	board(){}

	board(int digit_mask_positions[]){
		if (digit_mask_positions == NULL){
			is_valid = false;
			return;
		}

		for (int i = 0; i < 9; i++){
			this->digit_masks[i] = all_filters[i][digit_mask_positions[i]];

			for (int y = 0; y < 9; y++){
				for (int x = 0; x < 9; x++){
					if (BIT(all_filters[i][digit_mask_positions[i]], x, y)){
						current_game_num_y_map[i][y] = x;
						current_game[y][x] = i;
						break;
					}
				}
			}
		}
	}

	void play_move(move m, bool player_move = false){
		if (!m){
			std::cerr << "Trying to play an invalid move" << std::endl;
			return ;
		}

		std::cerr << "Playing move " << m.to_string() << std::endl;
		if (player_move) current_board_state[m.y][m.x] = m.num;

		moves_played++;
		current_game[m.y][m.x] = m.num;
		current_game_num_y_map[m.num][m.y] = m.x;
		digit_masks[m.num] |= ((i128)1 << BITMAP_OFFSET(m.x, m.y));
		digit_except_masks[m.num] |= ((row_mask << m.x) | (col_mask << (m.y * 9)) | (box_mask << BOX_OFFSET(m.x, m.y)));

		// Remove all the filters which are not valid anymore
		clean_filters(this);

		if (moves_played == 81){
			std::cerr << "Board solved, stop here" << std::endl;
			print_board("Right?");
			return ;
		}

		// Loop through all the masks, to create a map of all the possible digits for each cell
		// If all the valid masks for a cell are the same, we can play that move instantly
		for (int num = 0; num < 9; num++){
			i128 new_except_mask = 0b0;
			CREATE128(forced_moves, 0b1111111111111111111111111111111111111111111111111111111111111111, 0b1111111111111111111111111111111111111111111111111111111111111111)

			for (int i = 0; i < filter_count[num]; i++){
				new_except_mask |= all_filters[num][i];
				forced_moves &= all_filters[num][i];
			}

			digit_except_masks[num] |= (~new_except_mask | digit_masks[num]);
			forced_moves ^= digit_masks[num];

			if (forced_moves){
				for (int i = 0; i < 81; i++){
					if (forced_moves & ((i128)1 << i)){
						int x = i % 9;
						int y = i / 9;
						move forced = move(x, y, num);
						std::cerr << "Forced move: " << forced.to_string() << std::endl;
						play_move(forced);
						return ;
					}
				}	
			}
		}
	}

	void play_calculation_move(move m){
		if (!m){
			std::cerr << "Trying to play an invalid (calculation) move" << std::endl;
			return ;
		}

		// std::cerr << "Playing calculation move " << m.to_string() << std::endl;
		moves_played++;
		current_game[m.y][m.x] = m.num;
		current_game_num_y_map[m.num][m.y] = m.x;
		digit_masks[m.num] |= ((i128)1 << BITMAP_OFFSET(m.x, m.y));
		digit_except_masks[m.num] |= ((row_mask << m.x) | (col_mask << (m.y * 9)) | (box_mask << BOX_OFFSET(m.x, m.y)));

		// Remove all the filters which are not valid anymore
		prepare_board_specefic_filters(this);

		// Loop through all the masks, to create a map of all the possible digits for each cell
		// If all the valid masks for a cell are the same, we can play that move instantly
		for (int num = 0; num < 9; num++){
			i128 new_except_mask = 0b0;
			CREATE128(forced_moves, 0b1111111111111111111111111111111111111111111111111111111111111111, 0b1111111111111111111111111111111111111111111111111111111111111111)

			for (int i = 0; i < filter_count[num]; i++){
				if (!filters_active[num][i]) continue;
				new_except_mask |= all_filters[num][i];
				forced_moves &= all_filters[num][i];
			}

			digit_except_masks[num] |= (~new_except_mask | digit_masks[num]);
			forced_moves ^= digit_masks[num];

			if (forced_moves){
				for (int i = 0; i < 81; i++){
					if (forced_moves & ((i128)1 << i)){
						int x = i % 9;
						int y = i / 9;
						move forced = move(x, y, num);
						play_calculation_move(forced);
						return ;
					}
				}	
			}
		}
	}

	void print_board(std::string name = ""){
		std::cerr << name << std::endl;
		for (int y = 0; y < 9; y++){
			for (int x = 0; x < 9; x++){
				std::cerr << (current_game[y][x] == EMPTY ? '.' : (char)(current_game[y][x] + '1')) << " ";
			}
			std::cerr << std::endl;
		}
	}

	void print_digit_except_masks(){
		for (int i = 0; i < 9; i++){
			std::cerr << "Digit " << i + 1 << std::endl;
			print_i128_as_grid(digit_except_masks[i]);
		}
	}

	void print_digit_masks(){
		for (int i = 0; i < 9; i++){
			std::cerr << "Digit " << i + 1 << std::endl;
			print_i128_as_grid(digit_masks[i]);
		}
	}
};

void print_i128_as_grid(i128 input_num){
	for (int y = 0; y < 9; y++){
		for (int x = 0; x < 9; x++){
			std::cerr << (BIT(input_num, x, y) ? "1" : "0") << " ";
		}
		std::cerr << std::endl;
	}
	std::cerr << std::endl;
}

int calculate_filters(i128 current_board, int depth, int x_storage[]){
	if (depth == 9){
		i128 new_filter = 0b0;
		for (int i = 0; i < 9; i++)
			new_filter |= ((i128)1 << (x_storage[i] + i * 9));
		all_filters[0][filter_setup_index++] = new_filter;
		return (1);
	}

	int add = 0;
	for (int i = 0; i < 9; i++){
		if (BIT(current_board, i, depth)) continue;
		x_storage[depth] = i;
		i128 temp = current_board | (row_mask << BITMAP_OFFSET(i, depth)) | (box_mask << BOX_OFFSET(i, depth));
		add += calculate_filters(temp, depth + 1, x_storage);
	}

	return (filter_setup_index);
}

int prepare_filters(){
	int x_storage[9];
	if (!(calculate_filters(0b0, 0, x_storage) == AMOUNT_OF_FILTERS)) return (-1);
	for (int i = 1; i < 9; i++)
		std::memcpy(all_filters[i], all_filters[0], sizeof(all_filters[0]));
	return (AMOUNT_OF_FILTERS);
}

void clean_filters(board *b, int num){
	if (num == 9){
		return;
	}

	i128 temp_digit_mask = 0b0;
	for (int i = 0; i < 9; i++)
		if (i != num)
			temp_digit_mask |= b->digit_masks[i];

	int deleted = 0;
	for (int i = 0; i < filter_count[num]; i++){
		if ((b->digit_masks[num] & all_filters[num][i]) != b->digit_masks[num] || temp_digit_mask & all_filters[num][i]){
			deleted++;
			filter_count[num]--;
			all_filters[num][i] = all_filters[num][filter_count[num]];
			i--;
		}
	}

	clean_filters(b, num + 1);
}

void prepare_board_specefic_filters(board *b, int num){
	if (num == 9){
		return ;
	}

	i128 temp_digit_mask = 0b0;
	for (int i = 0; i < 9; i++)
		if (i != num)
			temp_digit_mask |= b->digit_masks[i];

	int count = 0;
	for (int i = 0; i < filter_count[num]; i++){
		filters_active[num][i] = !((b->digit_masks[num] & all_filters[num][i]) != b->digit_masks[num] || temp_digit_mask & all_filters[num][i]);
		if (!filters_active[num][i]){
			count++;
		}
	}

	prepare_board_specefic_filters(b, num + 1);
}

class algorithm{
public:
	algorithm(){}

	static void calculate_solve_order(){
		std::pair<int, int> order[9];
		for (int i = 0; i < 9; i++) order[i] = std::make_pair(filter_count[i], i);
		std::sort(order, order + 9);
		for (int i = 0; i < 9; i++) solve_order[i] = order[i].second;
	}

	static int basic_recursion(int depth, i128 current_board, int max_amount_of_solutions){
		if (depth == 9) return (1);

		int count = 0;
		int num = solve_order[depth];
		for (int i = 0; i < filter_count[num]; i++){
			if (all_filters[num][i] & current_board) continue;
			current_solution[num] = i;
			count += basic_recursion(depth + 1, current_board | all_filters[num][i], max_amount_of_solutions);
			if (count >= max_amount_of_solutions) return (count);
		}

		return (count);
	}

	static int custom_recursion(int depth, i128 current_board, int max_amount_of_solutions){
		if (depth == 9) return (1);

		int count = 0;
		int num = solve_order[depth];
		for (int i = 0; i < filter_count[num]; i++){
			if (all_filters[num][i] & current_board || !filters_active[num][i]) continue;
			current_solution[num] = i;
			count += custom_recursion(depth + 1, current_board | all_filters[num][i], max_amount_of_solutions);
			if (count >= max_amount_of_solutions) return (count);
		}

		return (count);
	}

	static int calculate_amount_of_solutions(int max_amount_of_solutions, bool use_custom_board = false){
		calculate_solve_order();
		return (use_custom_board ? custom_recursion(0, 0b0, max_amount_of_solutions) : basic_recursion(0, 0b0, max_amount_of_solutions));
	}

	static board calculate_xth_board(int x, bool use_custom_board = false){
		calculate_solve_order();
		int found_solutions = use_custom_board ? custom_recursion(0, 0b0, x) : basic_recursion(0, 0b0, x);
		if (found_solutions < x) return (NULL);
		return (board(current_solution));
	}

	static board get_most_recent_board(){
		return (board(current_solution));
	}

	static std::vector<move> calculate_possible_moves(board *base_board, board *new_board){
		std::vector<move> possible_moves;

		bool occupied[9][9] = {
			{base_board->current_game[0][0] != EMPTY, base_board->current_game[0][1] != EMPTY, base_board->current_game[0][2] != EMPTY, base_board->current_game[0][3] != EMPTY, base_board->current_game[0][4] != EMPTY, base_board->current_game[0][5] != EMPTY, base_board->current_game[0][6] != EMPTY, base_board->current_game[0][7] != EMPTY, base_board->current_game[0][8] != EMPTY},
			{base_board->current_game[1][0] != EMPTY, base_board->current_game[1][1] != EMPTY, base_board->current_game[1][2] != EMPTY, base_board->current_game[1][3] != EMPTY, base_board->current_game[1][4] != EMPTY, base_board->current_game[1][5] != EMPTY, base_board->current_game[1][6] != EMPTY, base_board->current_game[1][7] != EMPTY, base_board->current_game[1][8] != EMPTY},
			{base_board->current_game[2][0] != EMPTY, base_board->current_game[2][1] != EMPTY, base_board->current_game[2][2] != EMPTY, base_board->current_game[2][3] != EMPTY, base_board->current_game[2][4] != EMPTY, base_board->current_game[2][5] != EMPTY, base_board->current_game[2][6] != EMPTY, base_board->current_game[2][7] != EMPTY, base_board->current_game[2][8] != EMPTY},
			{base_board->current_game[3][0] != EMPTY, base_board->current_game[3][1] != EMPTY, base_board->current_game[3][2] != EMPTY, base_board->current_game[3][3] != EMPTY, base_board->current_game[3][4] != EMPTY, base_board->current_game[3][5] != EMPTY, base_board->current_game[3][6] != EMPTY, base_board->current_game[3][7] != EMPTY, base_board->current_game[3][8] != EMPTY},
			{base_board->current_game[4][0] != EMPTY, base_board->current_game[4][1] != EMPTY, base_board->current_game[4][2] != EMPTY, base_board->current_game[4][3] != EMPTY, base_board->current_game[4][4] != EMPTY, base_board->current_game[4][5] != EMPTY, base_board->current_game[4][6] != EMPTY, base_board->current_game[4][7] != EMPTY, base_board->current_game[4][8] != EMPTY},
			{base_board->current_game[5][0] != EMPTY, base_board->current_game[5][1] != EMPTY, base_board->current_game[5][2] != EMPTY, base_board->current_game[5][3] != EMPTY, base_board->current_game[5][4] != EMPTY, base_board->current_game[5][5] != EMPTY, base_board->current_game[5][6] != EMPTY, base_board->current_game[5][7] != EMPTY, base_board->current_game[5][8] != EMPTY},
			{base_board->current_game[6][0] != EMPTY, base_board->current_game[6][1] != EMPTY, base_board->current_game[6][2] != EMPTY, base_board->current_game[6][3] != EMPTY, base_board->current_game[6][4] != EMPTY, base_board->current_game[6][5] != EMPTY, base_board->current_game[6][6] != EMPTY, base_board->current_game[6][7] != EMPTY, base_board->current_game[6][8] != EMPTY},
			{base_board->current_game[7][0] != EMPTY, base_board->current_game[7][1] != EMPTY, base_board->current_game[7][2] != EMPTY, base_board->current_game[7][3] != EMPTY, base_board->current_game[7][4] != EMPTY, base_board->current_game[7][5] != EMPTY, base_board->current_game[7][6] != EMPTY, base_board->current_game[7][7] != EMPTY, base_board->current_game[7][8] != EMPTY},
			{base_board->current_game[8][0] != EMPTY, base_board->current_game[8][1] != EMPTY, base_board->current_game[8][2] != EMPTY, base_board->current_game[8][3] != EMPTY, base_board->current_game[8][4] != EMPTY, base_board->current_game[8][5] != EMPTY, base_board->current_game[8][6] != EMPTY, base_board->current_game[8][7] != EMPTY, base_board->current_game[8][8] != EMPTY}
		};

		for (int i = 0; i < 9; i++){
			for (int min = 0; min < 9; min++){
				for (int max = min + 1; max < 9; max++){
					int x_top = new_board->current_game_num_y_map[i][min];
					int x_bottom = new_board->current_game_num_y_map[i][max];
					int y_top = min;
					int y_bottom = max;

					if (occupied[y_top][x_top] || occupied[y_top][x_bottom] || occupied[y_bottom][x_top] || occupied[y_bottom][x_bottom]) continue;
					if ((x_top / 3 == x_bottom / 3) + (y_top / 3 == y_bottom / 3) != 1) continue;

					if (new_board->current_game[y_top][x_bottom] == new_board->current_game[y_bottom][x_top]){
						occupied[y_top][x_top] = true;
						occupied[y_top][x_bottom] = true;
						occupied[y_bottom][x_top] = true;
						occupied[y_bottom][x_bottom] = true;

						possible_moves.push_back(move(x_top, y_top, new_board->current_game[y_top][x_top]));
						possible_moves.push_back(move(x_top, y_top, new_board->current_game[y_top][x_bottom]));
					}
				}
			}
		}

		return (possible_moves);
	}

	static move get_move_to_end_game(board *b){
		bool searched[9][9][9];
		for (int num = 0; num < 9; num++){
			for (int y = 0; y < 9; y++){
				for (int x = 0; x < 9; x++){
					searched[num][y][x] = BIT(b->digit_except_masks[num], x, y);
				}
			}
		}

		for (int y = 0; y < 9; y++){
			for (int x = 0; x < 9; x++){
				if (b->current_game[y][x] == EMPTY) continue;
				for (int i = 0; i < 9; i++){
					searched[i][y][x] = true;
				}
			}
		}

		for (int num = 0; num < 9; num++){
			for (int y = 0; y < 9; y++){
				for (int x = 0; x < 9; x++){
					if (searched[num][y][x]) continue;
					search_count++;
					move move_to_check = move(x, y, num);
					board temp_board;
					std::memcpy(&temp_board, b, sizeof(board));
					temp_board.play_calculation_move(move_to_check);

					int amount_of_solutions = algorithm::calculate_amount_of_solutions(2, true);
					if (amount_of_solutions == 1 || temp_board.moves_played == 81){
						std::cerr << "moves played " << (int)temp_board.moves_played << std::endl;
						move_to_check.final_move = true;
						return (move_to_check);
					}
				}
			}
		}

		return (move());
	}

	static move get_random_valid_move(board *b){
		board solved_board = algorithm::calculate_xth_board(1);
		if (!solved_board.is_valid){
			std::cerr << "Board is not valid" << std::endl;
			return (move("Aa1"));
		}

		int x_offset = rand() % 9;
		int y_offset = rand() % 9;

		for (int y = 0; y < 9; y++){
			for (int x = 0; x < 9; x++){
				if (current_board_state[(y + y_offset) % 9][(x + x_offset) % 9] != EMPTY) continue;
				return (move((x + x_offset) % 9, (y + y_offset) % 9, solved_board.current_game[(y + y_offset) % 9][(x + x_offset) % 9]));
			}
		}

		std::cerr << "No valid move found - random" << std::endl;
		return (move("Aa1"));
	}

	static move calculate_bot_move(board *b){
		if (amount_of_moves_played < 8) return (get_random_valid_move(b));
		
		// Create a list with all the moves our program can play, which would decrease the amount of solutions
		std::vector<move> possible_moves;

		// Create a list with all the moves which were already calculated
		std::vector<move> moves_calculated;

		// Check if the board is already solved
		if (algorithm::calculate_amount_of_solutions(2) == 1){
			std::cerr << "Board already solved" << std::endl;
			move output_move = move(true);
			return (output_move);
		}

		// The board is not solved, so we need to calculate a possible solution
		int i = 0;
		while (++i < 30){
			// Calculate a possible solution for the sudoku
			board new_board = algorithm::calculate_xth_board(i);
			if (!new_board.is_valid) break;

			// Load in all the moves which would decrease the amount of solutions in the current board
			std::vector<move> moves_to_check = calculate_possible_moves(b, &new_board);

			// Play all the moves and check if one would result in a final solution
			for (move m : moves_to_check){
				bool stop = false;
				for (move m2 : moves_calculated){
					if (m == m2){
						total_moves_double++;
						stop = true;
					}

					if (stop) break;
				}
				if (stop) continue;

				total_moves_calculated++;
				moves_calculated.push_back(m);

				// Create a board with the move played
				// Note that this will also update the filters
				board temp_board;
				std::memcpy(&temp_board, b, sizeof(board));
				temp_board.play_calculation_move(m);

				// Calculate the amount of solutions for the new board
				int amount_of_solutions = algorithm::calculate_amount_of_solutions(2, true);
				if (amount_of_solutions == 1){
					m.final_move = true;
					return (m);
				}
				if (amount_of_solutions > 1){
					if (amount_of_moves_played > MOVE_COUNT_TO_EXTEND_THINKING && total_time + (double)(clock() - start) < MAX_TIMEFRAME_FOR_EXTEND_THINKING){
						move is_end_move = get_move_to_end_game(&temp_board);
						if (!is_end_move) return (m);
						possible_moves.push_back(m);
					}
					else {
						if (total_time + (double)(clock() - start) >= MAX_TIMEFRAME_FOR_EXTEND_THINKING) std::cerr << "Not enough time :(" << std::endl;
						return (m);
					}
				}
			}

			// std::cerr << possible_moves.size() << std::endl;
			if (possible_moves.size() > 500) break;
		}

		// If no moves were found to end the game, play a random move from all the moves which would decrease the amount of solutions
		if (possible_moves.size() > 0) return (possible_moves[rand() % possible_moves.size()]);

		// If no moves were found to decrease the amount of solutions, play a random move (TF is happening loll)
		std::cerr << "No possible moves found" << std::endl;
		int amount_of_solutions = algorithm::calculate_amount_of_solutions(2);
		if (amount_of_solutions == 0) std::cerr << "Whoopsie, unsolvable board" << std::endl;
		else if (amount_of_solutions == 1) std::cerr << "Board already solved" << std::endl;
		else std::cerr << "Board has " << amount_of_solutions << " solutions" << std::endl;

		return algorithm::get_random_valid_move(b);
	}
};

void load_board(board *b){
	for (int y = 0; y < 9; y++){
		std::string line;
		std::cin >> line;

		for (int x = 0; x < 9; x++){
			if (line[x] == '.') continue;
			b->play_move(move(x, y, line[x] - '1'));
		}
	}
}

int main(){
	// Setup random seed and turn off syncing
	unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	std::cerr << "Used seed: " << seed << std::endl;
	srand(seed);

	std::ios_base::sync_with_stdio(false);
	std::cin.tie(NULL);

	// Prepare the filters to calculate board solutions
	prepare_filters();

	move output_move;
	board main_board;

	std::string input;
	std::cin >> input;

	while (input != "Quit"){
		start = clock();

		// Handle move of opponent
		if (input != "Start"){
			main_board.play_move(move(input), true);
			amount_of_moves_played++;
		}

		// Handle our own move
		output_move = algorithm::calculate_bot_move(&main_board);
		main_board.play_move(output_move, true);
		amount_of_moves_played++;
		if (algorithm::calculate_amount_of_solutions(2) == 1) output_move.final_move = true;

		total_time += (double)(clock() - start);
		std::cerr << "Time used: " << total_time << std::endl;

		std::cout << output_move.to_string() << std::endl;
		std::cin >> input;
	}

	std::cerr << "Total moves calculated: " << total_moves_calculated << std::endl;
	std::cerr << "Total moves double: " << total_moves_double << std::endl;
	std::cerr << "Percentage: " << (double)total_moves_double / ((double)total_moves_calculated + (double)total_moves_double) * 100 << "%" << std::endl;
	std::cerr << "Search count: " << search_count << std::endl;

	main_board.print_board("End");

	return (0);
}