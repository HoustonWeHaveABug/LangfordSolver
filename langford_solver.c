#include <stdio.h>
#include <stdlib.h>

#define ORDER_MIN 2UL
#define RANGE_INF_MIN 1UL
#define DIMENSIONS_MIN 1UL
#define FLAG_COLOMBIANS_ONLY 1
#define FLAG_PLANARS_ONLY 2
#define FLAG_FIRST_ONLY 4
#define FLAG_VERBOSE 8
#define P_VAL_MAX 1000000000UL
#define P_DIGITS_MAX 9

typedef struct {
	unsigned long step;
	unsigned long start;
	unsigned long end;
	unsigned long dimension;
}
option_t;

typedef struct choice_s choice_t;

struct choice_s {
	unsigned long step;
	unsigned long start;
	choice_t *last;
	choice_t *next;
};

typedef struct {
	unsigned long value;
	unsigned long dimension;
}
number_t;

typedef struct node_s node_t;

struct node_s {
	union {
		unsigned long rows_n;
		node_t *column;
	};
	option_t *option;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

void usage(void);
int dlx_run(unsigned long (*)(unsigned long, unsigned long), unsigned long (*)(unsigned long), void (*)(unsigned long, unsigned long, unsigned long), void (*)(unsigned long, unsigned long), unsigned long, unsigned long, unsigned long);
void set_column(node_t *, node_t *);
void add_group_strict_half_options(unsigned long, unsigned long, unsigned long);
unsigned long set_group_strict_half_options(unsigned long, unsigned long);
void add_group_options(unsigned long, unsigned long);
unsigned long set_group_options(unsigned long);
void add_group_option(unsigned long, unsigned long, unsigned long);
unsigned long set_group_option(unsigned long, unsigned long);
void add_group_half_options(unsigned long, unsigned long);
unsigned long set_group_half_options(unsigned long);
void set_option(option_t *, unsigned long, unsigned long, unsigned long);
void add_row_nodes(option_t *);
void set_row_node(node_t *, option_t *, node_t *, node_t **);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(void);
void print_solution(void);
void print_number(number_t *);
void process_rows(node_t *, void (*)(node_t *));
void assign_row_with_conflicts(node_t *);
void chain_option(option_t *);
void set_choice(choice_t *, unsigned long, unsigned long, choice_t *, choice_t *);
void assign_row(node_t *);
void cover_row_columns(node_t *);
void cover_column(node_t *);
void cover_conflicts(option_t *);
int range_conflict(option_t *, unsigned long, unsigned long);
int range_inside(option_t *, unsigned long, unsigned long);
void cover_conflict(node_t *);
void cover_node(node_t *);
void assign_option(option_t *);
void set_number(number_t *, unsigned long, unsigned long);
void uncover_row_columns(node_t *);
void uncover_column(node_t *);
void uncover_row(node_t *);
void uncover_node(node_t *);
int compare_options(const void *, const void *);
int extend_trie(void);
void main_free(void);
int mp_new(mp_t *);
void mp_print(const char *, mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(mp_t *, unsigned long);
void mp_free(mp_t *);

int setting_planars_only, setting_colombians_only, setting_first_only, setting_verbose;
unsigned long order, intervals_n, range_inf, range_sup, hooks_n, numbers_n, columns_n, sentinel, dimensions_n, trie_branching, *trie, trie_size_max, trie_size;
option_t filler, *options_cur;
choice_t *choices, *choices_cur;
number_t *numbers;
node_t **tops, **top_first_group, *nodes, **conflicts_cur, *column_first_group, *column_sentinel, *header, *row_node;
mp_t cost, solutions_n;

int main(void) {
	int settings;
	unsigned long groups_n, group_sup_options_n;
	if (scanf("%lu", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %lu\n\n", ORDER_MIN);
		usage();
		return EXIT_FAILURE;
	}
	intervals_n = order-1UL;
	if (scanf("%lu", &range_inf) != 1 || range_inf < RANGE_INF_MIN) {
		fprintf(stderr, "Range inferior bound must be greater than or equal to %lu\n\n", RANGE_INF_MIN);
		usage();
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &range_sup) != 1 || range_sup < range_inf) {
		fprintf(stderr, "Range superior bound must be greater than or equal to Range inferior bound\n\n");
		usage();
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1UL;
	if (scanf("%lu", &hooks_n) != 1) {
		fprintf(stderr, "Error reading number of hooks\n\n");
		usage();
		return EXIT_FAILURE;
	}
	numbers_n = groups_n*order+hooks_n;
	columns_n = numbers_n+groups_n;
	if (scanf("%d", &settings) != 1) {
		fprintf(stderr, "Error reading setting flags\n\n");
		usage();
		return EXIT_FAILURE;
	}
	setting_colombians_only = settings & FLAG_COLOMBIANS_ONLY;
	setting_planars_only = settings & FLAG_PLANARS_ONLY;
	setting_first_only = settings & FLAG_FIRST_ONLY;
	setting_verbose = settings & FLAG_VERBOSE;
	if (setting_colombians_only) {
		if (scanf("%lu", &sentinel) != 1 || sentinel < range_inf || sentinel > range_sup) {
			fprintf(stderr, "Sentinel must lie between Range inferior bound and Range superior bound\n\n");
			usage();
			return EXIT_FAILURE;
		}
	}
	else {
		sentinel = range_inf;
	}
	if (setting_planars_only) {
		if (scanf("%lu", &dimensions_n) != 1 || dimensions_n < DIMENSIONS_MIN) {
			fprintf(stderr, "Number of dimensions must be greater than or equal to %lu\n\n", DIMENSIONS_MIN);
			usage();
			return EXIT_FAILURE;
		}
	}
	else {
		dimensions_n = DIMENSIONS_MIN;
	}
	set_option(&filler, 0UL, numbers_n, 0UL);
	if (setting_planars_only) {
		choices = malloc(sizeof(choice_t)*(groups_n+hooks_n+1UL));
		if (!choices) {
			fprintf(stderr, "Error allocating memory for choices\n");
			fflush(stderr);
			return EXIT_FAILURE;
		}
		set_choice(choices, 0UL, numbers_n, choices, choices);
		choices_cur = choices;
		trie_branching = groups_n+1UL;
		trie = malloc(sizeof(unsigned long)*trie_branching);
		if (!trie) {
			fprintf(stderr, "Error allocating memory for trie\n");
			fflush(stderr);
			free(choices);
			return EXIT_FAILURE;
		}
		trie_size_max = trie_branching;
	}
	if (setting_verbose) {
		numbers = malloc(sizeof(number_t)*numbers_n);
		if (!numbers) {
			fprintf(stderr, "Error allocating memory for numbers\n");
			fflush(stderr);
			if (setting_planars_only) {
				free(trie);
				free(choices);
			}
			return EXIT_FAILURE;
		}
	}
	tops = malloc(sizeof(node_t *)*columns_n);
	if (!tops) {
		fprintf(stderr, "Error allocating memory for tops\n");
		fflush(stderr);
		if (setting_verbose) {
			free(numbers);
		}
		if (setting_planars_only) {
			free(trie);
			free(choices);
		}
		return EXIT_FAILURE;
	}
	top_first_group = tops+numbers_n;
	if (!mp_new(&cost)) {
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		if (setting_planars_only) {
			free(trie);
			free(choices);
		}
		return EXIT_FAILURE;
	}
	if (!mp_new(&solutions_n)) {
		mp_free(&cost);
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		if (setting_planars_only) {
			free(trie);
			free(choices);
		}
		return EXIT_FAILURE;
	}
	group_sup_options_n = set_group_options(range_sup);
	if (!dlx_run(set_group_strict_half_options, set_group_options, add_group_strict_half_options, add_group_options, 0UL, 0UL, 0UL)) {
		main_free();
		return EXIT_FAILURE;
	}
	if (group_sup_options_n%2UL == 1UL && (!setting_first_only || mp_eq_val(&solutions_n, 0UL)) && !dlx_run(set_group_option, set_group_half_options, add_group_option, add_group_half_options, group_sup_options_n/2UL, 1UL, numbers_n/2UL+numbers_n%2UL)) {
		main_free();
		return EXIT_FAILURE;
	}
	mp_print("Final cost", &cost);
	mp_print("Solutions", &solutions_n);
	fflush(stdout);
	main_free();
	return EXIT_SUCCESS;
}

void usage(void) {
	fprintf(stderr, "Parameters read on standard input: order range_inf range_sup hooks_n settings [ sentinel ] [ dimensions_n ]\n\n");
	fprintf(stderr, "order must be greater than or equal to %lu\n", ORDER_MIN);
	fprintf(stderr, "range_inf = range inferior bound: must be greater than or equal to %lu\n", RANGE_INF_MIN);
	fprintf(stderr, "range_sup = range superior bound: must be greater than or equal to range_inf\n");
	fprintf(stderr, "hooks_n = number of hooks\n");
	fprintf(stderr, "settings = sum of option settings\n");
	fprintf(stderr, "- colombian solutions only = %d\n", FLAG_COLOMBIANS_ONLY);
	fprintf(stderr, "- planar solutions only = %d\n", FLAG_PLANARS_ONLY);
	fprintf(stderr, "- first solution only = %d\n", FLAG_FIRST_ONLY);
	fprintf(stderr, "- verbose mode = %d\n", FLAG_VERBOSE);
	fprintf(stderr, "sentinel: argument for the colombian variant\n");
	fprintf(stderr, "dimensions_n = number of dimensions: argument for the planar variant\n");
	fflush(stderr);
}

int dlx_run(unsigned long (*set_group_options_fn1)(unsigned long, unsigned long), unsigned long (*set_group_options_fn2)(unsigned long), void (*add_group_options_fn1)(unsigned long, unsigned long, unsigned long), void (*add_group_options_fn2)(unsigned long, unsigned long), unsigned long offset, unsigned long group_options_min, unsigned long hook_options_min) {
	unsigned long group_options_n = set_group_options_fn1(range_sup, offset), hook_options_n, nodes_n, i;
	option_t *options;
	node_t **conflicts;
	if (range_sup > range_inf) {
		group_options_n += set_group_options_fn2(range_sup-1UL)*dimensions_n;
		for (i = range_sup-2UL; i >= range_inf; i--) {
			group_options_n += set_group_options(i)*dimensions_n;
		}
	}
	if (hooks_n > 0UL) {
		if (group_options_n > group_options_min) {
			hook_options_n = numbers_n;
		}
		else {
			hook_options_n = hook_options_min;
		}
	}
	else {
		hook_options_n = 0UL;
	}
	options = malloc(sizeof(option_t)*(group_options_n+hook_options_n));
	if (!options) {
		fprintf(stderr, "Error allocating memory for options\n");
		fflush(stderr);
		return 0;
	}
	nodes_n = columns_n+1UL+group_options_n*(order+1UL)+hook_options_n;
	nodes = malloc(sizeof(node_t)*nodes_n);
	if (!nodes) {
		fprintf(stderr, "Error allocating memory for nodes\n");
		fflush(stderr);
		free(options);
		return 0;
	}
	if (setting_colombians_only || setting_planars_only) {
		conflicts = malloc(sizeof(node_t *)*group_options_n);
		if (!conflicts) {
			fprintf(stderr, "Error allocating memory for conflicts\n");
			fflush(stderr);
			free(nodes);
			free(options);
			return 0;
		}
		conflicts_cur = conflicts;
	}
	else {
		conflicts = NULL;
	}
	column_first_group = nodes+numbers_n;
	column_sentinel = column_first_group+sentinel-range_inf;
	header = nodes+columns_n;
	set_column(nodes, header);
	for (i = 0UL; i < columns_n; i++) {
		set_column(nodes+i+1UL, nodes+i);
		tops[i] = nodes+i;
	}
	options_cur = options;
	row_node = nodes+columns_n+1UL;
	add_group_options_fn1(range_sup, offset, 0UL);
	if (range_sup > range_inf) {
		for (i = 0UL; i < dimensions_n; i++) {
			add_group_options_fn2(range_sup-1UL, i);
		}
		for (i = range_sup-2UL; i >= range_inf; i--) {
			unsigned long j;
			for (j = 0UL; j < dimensions_n; j++) {
				add_group_options(i, j);
			}
		}
	}
	qsort(options, group_options_n, sizeof(option_t), compare_options);
	for (i = 0UL; i < group_options_n; i++) {
		add_row_nodes(options+i);
	}
	for (i = 0UL; i < hook_options_n; i++) {
		set_option(options_cur, 0UL, i, 0UL);
		set_row_node(nodes+i, options_cur, row_node, tops+i);
		options_cur++;
	}
	for (i = 0UL; i < columns_n; i++) {
		link_top(nodes+i, tops[i]);
	}
	dlx_search();
	if (setting_colombians_only || setting_planars_only) {
		free(conflicts);
	}
	free(nodes);
	free(options);
	return 1;
}

void set_column(node_t *column, node_t *left) {
	column->rows_n = 0UL;
	column->option = &filler;
	link_left(column, left);
}

void add_group_strict_half_options(unsigned long step, unsigned long offset, unsigned long dimension) {
	unsigned long group_options_n = set_group_strict_half_options(step, offset), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, dimension);
		options_cur++;
	}
}

unsigned long set_group_strict_half_options(unsigned long step, unsigned long offset) {
	return intervals_n*step+offset < numbers_n ? (numbers_n-intervals_n*step-offset)/2UL:0UL;
}

void add_group_options(unsigned long step, unsigned long dimension) {
	unsigned long group_options_n = set_group_options(step), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, dimension);
		options_cur++;
	}
}

unsigned long set_group_options(unsigned long step) {
	return intervals_n*step < numbers_n ? numbers_n-intervals_n*step:0UL;
}

void add_group_option(unsigned long step, unsigned long offset, unsigned long dimension) {
	unsigned long group_options_n = set_group_option(step, offset);
	if (group_options_n > 0UL) {
		set_option(options_cur, step, offset, dimension);
		options_cur++;
	}
}

unsigned long set_group_option(unsigned long step, unsigned long offset) {
	return intervals_n*step+offset < numbers_n ? 1UL:0UL;
}

void add_group_half_options(unsigned long step, unsigned long dimension) {
	unsigned long group_options_n = set_group_half_options(step), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, dimension);
		options_cur++;
	}
}

unsigned long set_group_half_options(unsigned long step) {
	return intervals_n*step < numbers_n ? (numbers_n-intervals_n*step)/2UL+(numbers_n-intervals_n*step)%2UL:0UL;
}

void set_option(option_t *option, unsigned long step, unsigned long start, unsigned long dimension) {
	option->step = step;
	option->start = start;
	option->end = start+step*intervals_n;
	option->dimension = dimension;
}

void add_row_nodes(option_t *option) {
	unsigned long i;
	set_row_node(nodes+option->start, option, row_node+order, tops+option->start);
	for (i = 1UL; i < order; i++) {
		set_row_node(nodes+option->start+i*option->step, option, row_node-1, tops+option->start+i*option->step);
	}
	set_row_node(column_first_group+option->step-range_inf, option, row_node-1, top_first_group+option->step-range_inf);
}

void set_row_node(node_t *column, option_t *option, node_t *left, node_t **top) {
	column->rows_n++;
	row_node->column = column;
	row_node->option = option;
	link_left(row_node, left);
	link_top(row_node, *top);
	*top = row_node++;
}

void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

void dlx_search(void) {
	node_t *column_min, *column;
	if (!mp_inc(&cost)) {
		return;
	}
	if (header->right == header) {
		if (setting_planars_only) {
			int unique = choices_cur == choices;
			unsigned long trie_pos = 0UL;
			choice_t *choice;
			for (choice = choices->next; choice != choices; choice = choice->next) {
				unsigned long branch_pos = trie_pos;
				if (choice->step > 0UL) {
					branch_pos += choice->step-range_inf+1UL;
				}
				if (trie[branch_pos] == 0UL) {
					if (choice->next == choices) {
						trie[branch_pos] = trie_pos;
					}
					else {
						if (!extend_trie()) {
							return;
						}
						trie[branch_pos] = trie_size-trie_branching;
					}
					unique++;
				}
				trie_pos = trie[branch_pos];
			}
			if (unique) {
				if (!mp_inc(&solutions_n)) {
					return;
				}
				print_solution();
			}
		}
		else {
			if (!mp_inc(&solutions_n)) {
				return;
			}
			print_solution();
		}
		return;
	}
	if (header->right->rows_n == 0UL) {
		return;
	}
	column_min = header->right;
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n < column_min->rows_n) {
			if (column->rows_n == 0UL) {
				return;
			}
			column_min = column;
		}
	}
	cover_column(column_min);
	if (setting_colombians_only || setting_planars_only) {
		process_rows(column_min, assign_row_with_conflicts);
	}
	else {
		process_rows(column_min, assign_row);
	}
	uncover_column(column_min);
}

void print_solution(void) {
	if (setting_verbose) {
		unsigned long i;
		mp_print("Cost", &cost);
		print_number(numbers);
		for (i = 1UL; i < numbers_n; i++) {
			putchar(' ');
			print_number(numbers+i);
		}
		puts("");
		fflush(stdout);
	}
}

void print_number(number_t *number) {
	printf("%lu", number->value);
	if (dimensions_n > DIMENSIONS_MIN) {
		printf("-%lu", number->dimension);
	}
}

void process_rows(node_t *column_min, void (*assign_row_fn)(node_t *)) {
	if (setting_first_only) {
		unsigned long half_rows_min = column_min->rows_n/2UL+column_min->rows_n%2UL, i;
		node_t *middle, *top, *bottom;
		for (i = 0UL, middle = column_min; i < half_rows_min; i++, middle = middle->bottom);
		if (column_min->rows_n%2UL == 1UL) {
			if (mp_eq_val(&solutions_n, 0UL)) {
				assign_row_fn(middle);
			}
			for (top = middle->top, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
				assign_row_fn(top);
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row_fn(bottom);
				}
			}
		}
		else {
			for (top = middle, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
				assign_row_fn(top);
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row_fn(bottom);
				}
			}
		}
	}
	else {
		node_t *row;
		for (row = column_min->bottom; row != column_min; row = row->bottom) {
			assign_row_fn(row);
		}
	}
}

void assign_row_with_conflicts(node_t *row) {
	node_t **conflicts_bak = conflicts_cur;
	if (setting_planars_only) {
		if (choices_cur > choices) {
			chain_option(row->option);
		}
		else {
			if (compare_options(row->top->option, row->option) == -1 || compare_options(row->option, row->bottom->option) == -1) {
				chain_option(row->option);
			}
			if (abs(compare_options(row->top->option, row->option)) > 1 && compare_options(row->option, row->bottom->option) == -1) {
				trie_size = 0UL;
				if (!extend_trie()) {
					return;
				}
			}
		}
	}
	cover_row_columns(row);
	cover_conflicts(row->option);
	assign_option(row->option);
	dlx_search();
	while (conflicts_cur != conflicts_bak) {
		conflicts_cur--;
		uncover_row(*conflicts_cur);
	}
	uncover_row_columns(row);
	if (setting_planars_only) {
		if (choices_cur > choices) {
			choices_cur->next->last = choices_cur->last;
			choices_cur->last->next = choices_cur->next;
			choices_cur--;
		}
	}
}

void chain_option(option_t *option) {
	choice_t *choice;
	choices_cur++;
	for (choice = choices->next; choice != choices && choice->start > option->start; choice = choice->next);
	choice->last->next = choices_cur;
	set_choice(choices_cur, option->step, option->start, choice->last, choice);
	choice->last = choices_cur;
}

void set_choice(choice_t *choice, unsigned long step, unsigned long start, choice_t *last, choice_t *next) {
	choice->step = step;
	choice->start = start;
	choice->last = last;
	choice->next = next;
}

void assign_row(node_t *row) {
	cover_row_columns(row);
	assign_option(row->option);
	dlx_search();
	uncover_row_columns(row);
}

void cover_row_columns(node_t *row) {
	node_t *node;
	for (node = row->right; node != row; node = node->right) {
		cover_column(node->column);
	}
}

void cover_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			cover_node(node);
		}
	}
}

void cover_conflicts(option_t *option) {
	unsigned long inf, sup, i;
	node_t *column_inf, *column;
	if (option->step == 0UL) {
		return;
	}
	inf = option->start;
	column_inf = nodes+inf;
	if (setting_colombians_only) {
		sup = option->end;
		if (option->step == sentinel) {
			node_t *column_sup = nodes+sup;
			for (column = header->right; column < column_inf; column = column->right);
			while (column < column_sup) {
				node_t *row;
				for (row = column->bottom; row != column; row = row->bottom) {
					if (!range_conflict(row->option, inf, sup)) {
						cover_conflict(row);
					}
				}
				column = column->right;
			}
		}
		else if (option->step < sentinel) {
			for (column = header->left; column != header && column > column_sentinel; column = column->left);
			if (column == column_sentinel) {
				node_t *row;
				for (row = column->bottom; row != column; row = row->bottom) {
					if (range_inside(row->option, inf, sup)) {
						cover_conflict(row);
					}
				}
				column = column->right;
			}
		}
	}
	if (setting_planars_only) {
		sup = inf+option->step;
		for (i = 1UL; i < order; i++) {
			node_t *column_sup = nodes+sup;
			for (column = header->right; column < column_inf; column = column->right);
			while (column < column_sup) {
				node_t *row;
				for (row = column->bottom; row != column; row = row->bottom) {
					if (row->option->dimension == option->dimension && range_conflict(row->option, inf, sup)) {
						cover_conflict(row);
					}
				}
				column = column->right;
			}
			inf = sup;
			sup += option->step;
		}
	}
}

int range_conflict(option_t *option, unsigned long inf, unsigned long sup) {
	return option->start < inf || option->end > sup;
}

int range_inside(option_t *option, unsigned long inf, unsigned long sup) {
	return option->start < inf && option->end > sup;
}

void cover_conflict(node_t *row) {
	node_t *node;
	cover_node(row);
	for (node = row->right; node != row; node = node->right) {
		cover_node(node);
	}
	*conflicts_cur = row;
	conflicts_cur++;
}

void cover_node(node_t *node) {
	node->column->rows_n--;
	node->bottom->top = node->top;
	node->top->bottom = node->bottom;
}

void assign_option(option_t *option) {
	if (setting_verbose) {
		if (option->step > 0UL) {
			unsigned long i;
			for (i = 0UL; i < order; i++) {
				set_number(numbers+option->start+i*option->step, option->step, option->dimension);
			}
		}
		else {
			set_number(numbers+option->start, option->step, option->dimension);
		}
	}
}

void set_number(number_t *number, unsigned long value, unsigned long dimension) {
	number->value = value;
	number->dimension = dimension;
}

void uncover_row_columns(node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_column(node->column);
	}
}

void uncover_column(node_t *column) {
	node_t *row;
	for (row = column->top; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			uncover_node(node);
		}
	}
	column->left->right = column;
	column->right->left = column;
}

void uncover_row(node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_node(node);
	}
	uncover_node(row);
}

void uncover_node(node_t *node) {
	node->top->bottom = node;
	node->bottom->top = node;
	node->column->rows_n++;
}

int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->start < option_b->start) {
		return -3;
	}
	if (option_a->start > option_b->start) {
		return 3;
	}
	if (option_a->step < option_b->step) {
		return 2;
	}
	if (option_a->step > option_b->step) {
		return -2;
	}
	if (option_a->dimension < option_b->dimension) {
		return -1;
	}
	return 1;
}

int extend_trie(void) {
	unsigned long i;
	if (trie_size == trie_size_max) {
		unsigned long *trie_tmp = realloc(trie, sizeof(unsigned long)*(trie_size_max+trie_branching));
		if (!trie_tmp) {
			fprintf(stderr, "Error reallocating memory for trie\n");
			fflush(stderr);
			return 0;
		}
		trie = trie_tmp;
		trie_size_max += trie_branching;
	}
	trie_size += trie_branching;
	for (i = trie_size-trie_branching; i < trie_size; i++) {
		trie[i] = 0UL;
	}
	return 1;
}

void main_free(void) {
	mp_free(&solutions_n);
	mp_free(&cost);
	free(tops);
	if (setting_verbose) {
		free(numbers);
	}
	if (setting_planars_only) {
		free(trie);
		free(choices);
	}
}

int mp_new(mp_t *mp) {
	mp->m = 1UL;
	mp->p = calloc(1UL, sizeof(unsigned long));
	if (!mp->p) {
		fprintf(stderr, "Error allocating memory for mp->p\n");
		fflush(stderr);
		return 0;
	}
	return 1;
}

void mp_print(const char *title, mp_t *mp) {
	unsigned long i;
	printf("%s %lu", title, mp->p[mp->m-1UL]);
	for (i = mp->m-1UL; i > 0UL; i--) {
		printf(",%0*lu", P_DIGITS_MAX, mp->p[i-1UL]);
	}
	puts("");
}

int mp_inc(mp_t *mp) {
	int carry;
	unsigned long i = 0UL;
	do {
		mp->p[i]++;
		carry = mp->p[i] == P_VAL_MAX;
		if (carry) {
			mp->p[i] = 0UL;
		}
		i++;
	}
	while (i < mp->m && carry);
	if (carry) {
		unsigned long *p = realloc(mp->p, sizeof(unsigned long)*(mp->m+1UL));
		if (!p) {
			fprintf(stderr, "Error reallocating memory for mp->p\n");
			fflush(stderr);
			return 0;
		}
		mp->p = p;
		mp->p[mp->m++] = 1UL;
	}
	return 1;
}

int mp_eq_val(mp_t *mp, unsigned long val) {
	return mp->m == 1UL && mp->p[0] == val;
}

void mp_free(mp_t *mp) {
	free(mp->p);
}
