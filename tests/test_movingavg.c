#include <stdio.h>
#include <minunit.h>
#include <movingavg.h>

int tests_run = 0;

static char *test_create_and_destroy() {
	struct mavg_t *m = mavg_new(300);
	mu_assert("mavg_new returned null", m != NULL);
	mavg_free(m);
	return 0;
}

static char *test_correctness_empty() {
	struct mavg_t *m = mavg_new(10);
	uint16_t a = mavg_get(m);
	mu_assert("mavg_get on an empty buffer is supposed to return zero.", a == 0);
	return 0;
}

static char *test_correctness_constant() {
	static uint16_t constant_sample = 7;
	struct mavg_t *m = mavg_new(2);
	mavg_push_sample(m, constant_sample);
	uint16_t a = mavg_get(m);
	mu_assert("mavg_t should not be full.", m->full == false);
	mu_assert("For a mavg_t with one sample, mavg_get should return the sample.", a == constant_sample);
	return 0;
}

static char *test_correctness_full_constant() {
	static uint16_t buffer_length = 7;
	static uint16_t constant_sample = 7;
	struct mavg_t *m = mavg_new(buffer_length);

	for (uint16_t n = 0; n < buffer_length; n++) {
		mavg_push_sample(m, constant_sample);
	}

	uint16_t a = mavg_get(m);
	mu_assert("mavg_t should be full.", m->full == true);
	mu_assert("For a mavg_t with one sample, mavg_get should return the sample.", a == constant_sample);
	return 0;
}

static char *test_correctness_rounding() {
	struct mavg_t *m = mavg_new(300);
	
	mavg_push_sample(m, 7);
	mu_assert("mavg_get() should have returned 7", mavg_get(m) == 7);

	mavg_push_sample(m, 2);
	mu_assert("mavg_get() should have returned 4", mavg_get(m) == 4);

	mavg_push_sample(m, 2);
	mu_assert("mavg_get() should have returned 3", mavg_get(m) == 3);

	mavg_push_sample(m, 100);
	mu_assert("mavg_get() should have returned 27", mavg_get(m) == 27);
	return 0;
}

static char * all_tests() {
	mu_run_test(test_create_and_destroy);
	mu_run_test(test_correctness_empty);
	mu_run_test(test_correctness_constant);
	mu_run_test(test_correctness_full_constant);
	mu_run_test(test_correctness_rounding);
	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
