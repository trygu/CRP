// load_crp0old must accept sum sections that exceed the legacy 30*nsums
// buffer. Tables with 1000 or more cells used a fixed cap of 30 terms
// per sum, so margins wider than 30 cells (e.g. a 30x40 table with a
// Total column: 40 terms per row sum) were rejected with "ERROR:
// loading CRP instance" and RND failed for integer and fractional
// tables alike.

#include <cstdio>

extern "C" int CRPtest_load0old(const char *path, int *nterms_out);

static int failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++failures; \
        } \
    } while (0)

static int write_jj(const char *path, int ncells, const char *sum_line)
{
    FILE *f = fopen(path, "w");
    if (!f) {
        return -1;
    }
    std::fprintf(f, "0\n%d\n", ncells);
    for (int i = 0; i < ncells; i++) {
        std::fprintf(f, "%d 0.0 1 s 0.0 10000.0 0.0 0.0 0.0\n", i);
    }
    std::fprintf(f, "1\n");
    std::fputs(sum_line, f);
    fclose(f);
    return 0;
}

int main()
{
    // ncells = 1000 selects the legacy 30*nsums branch; a 31-term
    // sum (margin + 30 members) used to trip j > maxsize.
    char sum[4096];
    int off = std::sprintf(sum, "0 31 : 0 (-1)");
    for (int i = 1; i <= 30; i++) {
        off += std::sprintf(sum + off, " %d (1)", i);
    }
    off += std::sprintf(sum + off, "\n");
    (void)off;
    CHECK(write_jj("crp_maxsize_wide.jj", 1000, sum) == 0);
    int nterms = -1;
    CHECK(CRPtest_load0old("crp_maxsize_wide.jj", &nterms) == 0);
    CHECK(nterms == 31);
    remove("crp_maxsize_wide.jj");

    // Regression guard: a small instance (ncells*nsums branch) still
    // loads.
    CHECK(write_jj("crp_maxsize_small.jj", 4,
                   "0 4 : 0 (-1) 1 (1) 2 (1) 3 (1)\n") == 0);
    nterms = -1;
    CHECK(CRPtest_load0old("crp_maxsize_small.jj", &nterms) == 0);
    CHECK(nterms == 4);
    remove("crp_maxsize_small.jj");

    if (failures == 0) {
        std::printf("PASS crp_maxsize\n");
    }
    return failures ? 1 : 0;
}
