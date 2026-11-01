#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

static int write_answer(char *out, size_t cap,
                        const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used,
                            "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t a = (low + 0x4b8u) ^ high;
    uint32_t b = (low + 4u * mid) ^ 0xa64au;

    uint32_t values[MAX_FIELDS] = {0};
    values[0] = (3u * b + a) ^ 0x799u;
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = (uint32_t)(challenge & 15u);

    for (unsigned i = 1; i < 6; ++i) {
        unsigned shift = 18u + 5u * (i - 1u);
        uint32_t part = (uint32_t)((challenge >> shift) & 31u);
        values[i] = ((2u * values[i - 1]) ^ part) + 9u * i;
    }

    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)(challenge & 7u);
    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)(challenge >> 3);

    values[0] = index;

    switch (index) {
        case 0:
            values[1] = (x ^ 0x5d46u) + (x >> 1);
            break;
        case 1:
            values[1] = (4u * x) ^ 0xa7c1u;
            break;
        case 2:
            values[1] = (4u * x) ^ 0xfc4du;
            break;
        case 3:
            values[1] = (x ^ 0x1836u) + (x >> 1);
            break;
        case 4:
            values[1] = 5u * x + 0x8fbau;
            break;
        case 5:
            values[1] = 5u * x + 0xfbdcu;
            break;
        case 6:
            values[1] = 5u * x + 0x2b520u;
            break;
        case 7:
            values[1] = 5u * x + 0x221fu;
            break;
    }

    return write_answer(out, cap, values, 2);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low,
                            uint32_t high, uint32_t key) {
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid) {
        return (mid + 0x9cu) ^ key;
    }

    if (target < mid) {
        uint32_t child = phase4_walk(target, low, mid - 1u, key);
        return (5u * child) ^ (mid + key) ^ 0x18u;
    }

    uint32_t child = phase4_walk(target, mid + 1u, high, key);
    return (5u * child) ^ (mid + key) ^ 0x1du;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t target = (uint32_t)challenge & 127u;
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);

    uint32_t values[MAX_FIELDS] = {0};
    values[0] = phase4_walk(target, 0u, 127u, key);
    return write_answer(out, cap, values, 1);
}

struct Phase5Node {
    uint32_t value;
    uint32_t id;
    int next;
};

static const struct Phase5Node phase5_nodes[12] = {
    {0xfc1fu, 0xb2au,  8},
    {0xf2f0u, 0x450u, 11},
    {0x12f3u, 0x272u, -1},
    {0xb464u, 0x835u,  1},
    {0x7410u, 0xa31u,  7},
    {0x9edcu, 0x37fu,  4},
    {0x3f44u, 0x547u,  5},
    {0x8134u, 0x02cu,  3},
    {0xbc76u, 0x95cu,  9},
    {0x3f28u, 0x667u,  2},
    {0x85d8u, 0x728u,  0},
    {0xc0b4u, 0x11bu, 10}
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const int heads[8] = {6, 5, 4, 7, 3, 1, 11, 10};

    uint32_t values[MAX_FIELDS] = {0};
    unsigned steps = (unsigned)((challenge >> 3) & 3u);
    int node = heads[challenge & 7u];

    uint32_t acc = (uint32_t)(challenge >> 6)
                 ^ (uint32_t)(challenge >> 32)
                 ^ 0xfbd4u;

    for (unsigned i = 0; i <= steps; ++i) {
        if (node < 0 || node >= 12) return 1;

        acc = (3u * acc + phase5_nodes[node].value)
            ^ phase5_nodes[node].id;
        values[1] = phase5_nodes[node].id;

        if (i < steps) {
            node = phase5_nodes[node].next;
        }
    }

    values[0] = acc;
    return write_answer(out, cap, values, 2);
}

static uint32_t phase6_apply(unsigned index, uint32_t x) {
    static const unsigned ops[8] = {0, 2, 5, 0, 2, 4, 3, 1};

    switch (ops[index]) {
        case 0:
            return ((x >> 3) ^ x) + 0xcd83u;
        case 1:
            return ((x << 3) | (x >> 29)) ^ 0x8f9cu;
        case 2:
            return (x ^ 0x3cefu) + 4u * x;
        case 3:
            return (x + 0xa26du) ^ (x >> 1);
        case 4:
            return (x >> 3) | (x << 29);
        case 5:
            return (4u * x + 0x1a98u) ^ (7u * x);
    }

    return 0;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    unsigned first = (unsigned)(challenge & 7u);
    unsigned second = (unsigned)((challenge >> 3) & 7u);

    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);

    uint32_t intermediate = phase6_apply(first, x) ^ 0x774u;

    uint32_t values[MAX_FIELDS] = {0};
    values[0] = phase6_apply(second, intermediate) + 0xc562u;
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    return 1;
}