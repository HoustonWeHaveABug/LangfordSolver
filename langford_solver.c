#include <stdio.h>
#include <stdlib.h>

#define ORDER_MIN 2UL
#define RANGE_INF_MIN 1UL
#define FLAG_PLANARS_ONLY 1
#define FLAG_FIRST_ONLY 2
#define FLAG_VERBOSE 4
#define SIDE_NORTH 0UL
#define SIDE_SOUTH 1UL
#define P_VAL_MAX 1000000000UL
#define P_DIGITS_MAX 9

typedef struct {
	unsigned long step;
	unsigned long start;
	unsigned long side;
}
choice_t;

typedef struct node_s node_t;

struct node_s {
	union {
		unsigned long rows_n;
		node_t *column;
	};
	choice_t *choice;
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

unsigned long set_group_choices(unsigned long);
unsigned long set_group_half_choices(unsigned long);
void set_column(node_t *, node_t *);
void add_group_strict_half_choices(unsigned long, unsigned long, unsigned long);
void add_group_choices(unsigned long, unsigned long, unsigned long);
void add_group_half_choices(unsigned long, unsigned long, unsigned long);
void set_choice_cur(unsigned long, unsigned long, unsigned long);
int compare_choices(const void *, const void *);
void add_row_nodes(choice_t *);
void set_row_node(node_t *, choice_t *, node_t *, node_t **);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(void);
void assign_row_planars_only(node_t *);
void assign_row(node_t *);
void cover_row_columns(node_t *);
void cover_column(node_t *);
void check_conflicts(choice_t *);
int in_conflict(choice_t *, unsigned long, unsigned long, unsigned long);
void cover_row(node_t *);
void cover_node(node_t *);
void assign_choice(choice_t *);
void uncover_row_columns(node_t *);
void uncover_column(node_t *);
void uncover_row(node_t *);
void uncover_node(node_t *);
int mp_new(mp_t *);
void mp_print(const char *, mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(mp_t *, unsigned long);
void mp_free(mp_t *);

int option_planars_only, option_first_only, option_verbose;
unsigned long order, intervals_n, range_inf, range_sup, sequence_size, *sequence;
choice_t *choice_cur;
node_t **tops, *nodes, **conflicts, **conflicts_cur, *header, *row_node;
mp_t cost, solutions_n;

int main(void) {
	int options;
	unsigned long groups_n, hooks_n, columns_n, group_sup_choices_n, group_choices_n, hook_choices_n, nodes_n, i;
	choice_t *choices;
	if (scanf("%lu", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %lu\n", ORDER_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	intervals_n = order-1UL;
	if (scanf("%lu", &range_inf) != 1 || range_inf < RANGE_INF_MIN) {
		fprintf(stderr, "Range inferior bound must be greater than or equal to %lu\n", RANGE_INF_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &range_sup) != 1 || range_sup < range_inf) {
		fprintf(stderr, "Range superior bound must be greater than or equal to Range inferior bound\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1UL;
	if (scanf("%lu", &hooks_n) != 1) {
		fprintf(stderr, "Error reading number of hooks\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	sequence_size = groups_n*order+hooks_n;
	sequence = malloc(sizeof(unsigned long)*sequence_size);
	if (!sequence) {
		fprintf(stderr, "Error allocating memory for sequence\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	columns_n = sequence_size+groups_n;
	tops = malloc(sizeof(node_t *)*columns_n);
	if (!tops) {
		fprintf(stderr, "Error allocating memory for tops\n");
		fflush(stderr);
		free(sequence);
		return EXIT_FAILURE;
	}
	if (scanf("%d", &options) != 1) {
		fprintf(stderr, "Error reading option flags\n");
		fflush(stderr);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	option_planars_only = options & FLAG_PLANARS_ONLY;
	option_first_only = options & FLAG_FIRST_ONLY;
	option_verbose = options & FLAG_VERBOSE;
	if (!mp_new(&cost)) {
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	if (!mp_new(&solutions_n)) {
		mp_free(&cost);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	group_sup_choices_n = set_group_choices(intervals_n*range_sup);
	group_choices_n = group_sup_choices_n/2UL;
	if (option_planars_only) {
		for (i = range_sup-1UL; i >= range_inf; i--) {
			group_choices_n += set_group_choices(intervals_n*i)*2UL;
		}
	}
	else {
		for (i = range_sup-1UL; i >= range_inf; i--) {
			group_choices_n += set_group_choices(intervals_n*i);
		}
	}
	if (hooks_n > 0UL) {
		hook_choices_n = sequence_size;
	}
	else {
		hook_choices_n = 0UL;
	}
	choices = malloc(sizeof(choice_t)*(group_choices_n+hook_choices_n));
	if (!choices) {
		fprintf(stderr, "Error allocating memory for choices\n");
		fflush(stderr);
		mp_free(&solutions_n);
		mp_free(&cost);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	nodes_n = columns_n+1UL+group_choices_n*(order+1UL)+hook_choices_n;
	nodes = malloc(sizeof(node_t)*nodes_n);
	if (!nodes) {
		fprintf(stderr, "Error allocating memory for nodes\n");
		fflush(stderr);
		free(choices);
		mp_free(&solutions_n);
		mp_free(&cost);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	if (option_planars_only) {
		conflicts = malloc(sizeof(node_t *)*(group_choices_n+hook_choices_n));
		if (!conflicts) {
			fprintf(stderr, "Error allocating memory for conflicts\n");
			fflush(stderr);
			free(nodes);
			free(choices);
			mp_free(&solutions_n);
			mp_free(&cost);
			free(tops);
			free(sequence);
			return EXIT_FAILURE;
		}
		conflicts_cur = conflicts;
	}
	header = nodes+columns_n;
	set_column(nodes, header);
	for (i = 0UL; i < columns_n; i++) {
		set_column(nodes+i+1UL, nodes+i);
		tops[i] = nodes+i;
	}
	choice_cur = choices;
	row_node = nodes+columns_n+1UL;
	add_group_strict_half_choices(range_sup, intervals_n*range_sup, SIDE_NORTH);
	for (i = range_sup-1UL; i >= range_inf; i--) {
		add_group_choices(i, intervals_n*i, SIDE_NORTH);
		if (option_planars_only) {
			add_group_choices(i, intervals_n*i, SIDE_SOUTH);
		}
	}
	qsort(choices, group_choices_n, sizeof(choice_t), compare_choices);
	for (i = 0UL; i < group_choices_n; i++) {
		add_row_nodes(choices+i);
	}
	for (i = 0UL; i < hook_choices_n; i++) {
		set_choice_cur(0UL, i, SIDE_NORTH);
		set_row_node(nodes+i, choice_cur, row_node, tops+i);
		choice_cur++;
	}
	for (i = 0UL; i < columns_n; i++) {
		link_top(nodes+i, tops[i]);
	}
	dlx_search();
	if (option_planars_only) {
		free(conflicts);
	}
	free(nodes);
	free(choices);
	if (group_sup_choices_n%2UL == 1UL && (!option_first_only || mp_eq_val(&solutions_n, 0UL))) {
		group_choices_n = 1UL;
		if (range_sup > range_inf) {
			if (option_planars_only) {
				group_choices_n += set_group_half_choices(intervals_n*(range_sup-1UL))*2UL;
				for (i = range_sup-2UL; i >= range_inf; i--) {
					group_choices_n += set_group_choices(intervals_n*i)*2UL;
				}
			}
			else {
				group_choices_n += set_group_half_choices(intervals_n*(range_sup-1UL));
				for (i = range_sup-2UL; i >= range_inf; i--) {
					group_choices_n += set_group_choices(intervals_n*i);
				}
			}
		}
		if (hooks_n > 0UL) {
			if (group_choices_n > 1UL) {
				hook_choices_n = sequence_size;
			}
			else {
				hook_choices_n = sequence_size/2UL+sequence_size%2UL;
			}
		}
		else {
			hook_choices_n = 0UL;
		}
		choices = malloc(sizeof(choice_t)*(group_choices_n+hook_choices_n));
		if (!choices) {
			fprintf(stderr, "Error allocating memory for choices\n");
			fflush(stderr);
			mp_free(&solutions_n);
			mp_free(&cost);
			free(tops);
			free(sequence);
			return EXIT_FAILURE;
		}
		nodes_n = columns_n+1UL+group_choices_n*(order+1UL)+hook_choices_n;
		nodes = malloc(sizeof(node_t)*nodes_n);
		if (!nodes) {
			fprintf(stderr, "Error allocating memory for nodes\n");
			fflush(stderr);
			free(choices);
			mp_free(&solutions_n);
			mp_free(&cost);
			free(tops);
			free(sequence);
			return EXIT_FAILURE;
		}
		if (option_planars_only) {
			conflicts = malloc(sizeof(node_t *)*(group_choices_n+hook_choices_n));
			if (!conflicts) {
				fprintf(stderr, "Error allocating memory for conflicts\n");
				fflush(stderr);
				free(nodes);
				free(choices);
				mp_free(&solutions_n);
				mp_free(&cost);
				free(tops);
				free(sequence);
				return EXIT_FAILURE;
			}
			conflicts_cur = conflicts;
		}
		header = nodes+columns_n;
		set_column(nodes, header);
		for (i = 0UL; i < columns_n; i++) {
			set_column(nodes+i+1UL, nodes+i);
			tops[i] = nodes+i;
		}
		choice_cur = choices;
		row_node = nodes+columns_n+1UL;
		set_choice_cur(range_sup, group_sup_choices_n/2UL, SIDE_NORTH);
		choice_cur++;
		if (range_sup > range_inf) {
			add_group_half_choices(range_sup-1UL, intervals_n*(range_sup-1UL), SIDE_NORTH);
			if (option_planars_only) {
				add_group_half_choices(range_sup-1UL, intervals_n*(range_sup-1UL), SIDE_SOUTH);
			}
			for (i = range_sup-2UL; i >= range_inf; i--) {
				add_group_choices(i, intervals_n*i, SIDE_NORTH);
				if (option_planars_only) {
					add_group_choices(i, intervals_n*i, SIDE_SOUTH);
				}
			}
		}
		qsort(choices, group_choices_n, sizeof(choice_t), compare_choices);
		for (i = 0UL; i < group_choices_n; i++) {
			add_row_nodes(choices+i);
		}
		for (i = 0UL; i < hook_choices_n; i++) {
			set_choice_cur(0UL, i, SIDE_NORTH);
			set_row_node(nodes+i, choice_cur, row_node, tops+i);
			choice_cur++;
		}
		for (i = 0UL; i < columns_n; i++) {
			link_top(nodes+i, tops[i]);
		}
		dlx_search();
		if (option_planars_only) {
			free(conflicts);
		}
		free(nodes);
		free(choices);
	}
	mp_print("Final cost", &cost);
	mp_print("Solutions", &solutions_n);
	fflush(stdout);
	mp_free(&solutions_n);
	mp_free(&cost);
	free(tops);
	free(sequence);
	return EXIT_SUCCESS;
}

unsigned long set_group_choices(unsigned long group_span) {
	return group_span < sequence_size ? sequence_size-group_span:0UL;
}

unsigned long set_group_half_choices(unsigned long group_span) {
	return group_span < sequence_size ? (sequence_size-group_span)/2UL+(sequence_size-group_span)%2UL:0UL;
}

void set_column(node_t *node, node_t *left) {
	node->rows_n = 0UL;
	link_left(node, left);
}

void add_group_strict_half_choices(unsigned long step, unsigned long group_span, unsigned long side) {
	unsigned long group_choices_n = set_group_choices(group_span)/2UL, i;
	for (i = 0UL; i < group_choices_n; i++) {
		set_choice_cur(step, i, side);
		choice_cur++;
	}
}

void add_group_choices(unsigned long step, unsigned long group_span, unsigned long side) {
	unsigned long group_choices_n = set_group_choices(group_span), i;
	for (i = 0UL; i < group_choices_n; i++) {
		set_choice_cur(step, i, side);
		choice_cur++;
	}
}

void add_group_half_choices(unsigned long step, unsigned long group_span, unsigned long side) {
	unsigned long group_choices_n = set_group_half_choices(group_span), i;
	for (i = 0UL; i < group_choices_n; i++) {
		set_choice_cur(step, i, side);
		choice_cur++;
	}
}

void set_choice_cur(unsigned long step, unsigned long start, unsigned long side) {
	choice_cur->step = step;
	choice_cur->start = start;
	choice_cur->side = side;
}

int compare_choices(const void *a, const void *b) {
	const choice_t *choice_a = (const choice_t *)a, *choice_b = (const choice_t *)b;
	if (choice_a->start < choice_b->start) {
		return -1;
	}
	if (choice_a->start > choice_b->start) {
		return 1;
	}
	if (choice_a->step < choice_b->step) {
		return 1;
	}
	if (choice_a->step > choice_b->step) {
		return -1;
	}
	return 0;
}

void add_row_nodes(choice_t *choice) {
	unsigned long i;
	set_row_node(nodes+choice->start, choice, row_node+order, tops+choice->start);
	for (i = 1UL; i < order; i++) {
		set_row_node(nodes+choice->start+i*choice->step, choice, row_node-1, tops+choice->start+i*choice->step);
	}
	set_row_node(nodes+sequence_size+choice->step-range_inf, choice, row_node-1, tops+sequence_size+choice->step-range_inf);
}

void set_row_node(node_t *column, choice_t *choice, node_t *left, node_t **top) {
	column->rows_n++;
	row_node->column = column;
	row_node->choice = choice;
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
	unsigned long i;
	node_t *column_min, *column;
	if (!mp_inc(&cost)) {
		return;
	}
	if (header->right == header) {
		if (!mp_inc(&solutions_n)) {
			return;
		}
		if (option_verbose) {
			mp_print("Cost", &cost);
			if (option_planars_only) {
				if (sequence[0] <= range_sup) {
					printf("%lun", sequence[0]);
				}
				else {
					printf("%lus", sequence[0]-range_sup);
				}
				for (i = 1UL; i < sequence_size; i++) {
					if (sequence[i] <= range_sup) {
						printf(" %lun", sequence[i]);
					}
					else {
						printf(" %lus", sequence[i]-range_sup);
					}
				}
			}
			else {
				printf("%lu", sequence[0]);
				for (i = 1UL; i < sequence_size; i++) {
					printf(" %lu", sequence[i]);
				}
			}
			puts("");
			fflush(stdout);
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
	if (option_first_only) {
		unsigned long half_rows_min = column_min->rows_n/2UL+column_min->rows_n%2UL;
		node_t *middle, *top, *bottom;
		for (i = 0UL, middle = column_min; i < half_rows_min; i++, middle = middle->bottom);
		if (option_planars_only) {
			if (column_min->rows_n%2UL == 1UL) {
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row_planars_only(middle);
				}
				for (top = middle->top, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
					assign_row_planars_only(top);
					if (mp_eq_val(&solutions_n, 0UL)) {
						assign_row_planars_only(bottom);
					}
				}
			}
			else {
				for (top = middle, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
					assign_row_planars_only(top);
					if (mp_eq_val(&solutions_n, 0UL)) {
						assign_row_planars_only(bottom);
					}
				}
			}
		}
		else {
			if (column_min->rows_n%2UL == 1UL) {
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row(middle);
				}
				for (top = middle->top, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
					assign_row(top);
					if (mp_eq_val(&solutions_n, 0UL)) {
						assign_row(bottom);
					}
				}
			}
			else {
				for (top = middle, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
					assign_row(top);
					if (mp_eq_val(&solutions_n, 0UL)) {
						assign_row(bottom);
					}
				}
			}
		}
	}
	else {
		node_t *row;
		if (option_planars_only) {
			for (row = column_min->bottom; row != column_min; row = row->bottom) {
				assign_row_planars_only(row);
			}
		}
		else {
			for (row = column_min->bottom; row != column_min; row = row->bottom) {
				assign_row(row);
			}
		}
	}
	uncover_column(column_min);
}

void assign_row_planars_only(node_t *row) {
	node_t **conflicts_bak = conflicts_cur;
	cover_row_columns(row);
	check_conflicts(row->choice);
	assign_choice(row->choice);
	dlx_search();
	while (conflicts_cur != conflicts_bak) {
		conflicts_cur--;
		uncover_row(*conflicts_cur);
	}
	uncover_row_columns(row);
}

void assign_row(node_t *row) {
	cover_row_columns(row);
	assign_choice(row->choice);
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

void check_conflicts(choice_t *choice) {
	unsigned long a, b, i;
	node_t *column_a, *column;
	if (choice->step == 0UL) {
		return;
	}
	a = choice->start;
	b = a+choice->step;
	column_a = nodes+a;
	for (column = header->right; column < column_a; column = column->right);
	for (i = 1UL; i < order; i++) {
		node_t *column_b = nodes+b;
		while (column < column_b) {
			node_t *row;
			for (row = column->bottom; row != column; row = row->bottom) {
				if (in_conflict(row->choice, choice->side, a, b)) {
					cover_row(row);
					*conflicts_cur = row;
					conflicts_cur++;
				}
			}
			column = column->right;
		}
		a = b;
		b += choice->step;
	}
}

int in_conflict(choice_t *choice, unsigned long side, unsigned long a, unsigned long b) {
	return choice->side == side && (choice->start < a || choice->start+choice->step*intervals_n > b);
}

void cover_row(node_t *row) {
	node_t *node;
	cover_node(row);
	for (node = row->right; node != row; node = node->right) {
		cover_node(node);
	}
}

void cover_node(node_t *node) {
	node->column->rows_n--;
	node->bottom->top = node->top;
	node->top->bottom = node->bottom;
}

void assign_choice(choice_t *choice) {
	if (choice->step > 0UL) {
		unsigned long i;
		for (i = 0UL; i < order; i++) {
			sequence[choice->start+i*choice->step] = choice->step+range_sup*choice->side;
		}
	}
	else {
		sequence[choice->start] = 0UL;
	}
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
