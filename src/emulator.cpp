#include <dbg.h>

// z80ex callback signatures (must match z80ex.h)
static uint8_t memread_cb(Z80EX_CONTEXT *, uint16_t addr, int /*m1_state*/, void *user_data)
{
    auto *dbg_ptr = static_cast<dbg *>(user_data);
    return dbg_ptr->memory()[addr];
}

static void memwrite_cb(Z80EX_CONTEXT *, uint16_t addr, uint8_t value, void *user_data)
{
    auto *dbg_ptr = static_cast<dbg *>(user_data);
    dbg_ptr->memory()[addr] = value;
}

static uint8_t portread_cb(Z80EX_CONTEXT *, uint16_t, void *) { return 0xFF; }

static void portwrite_cb(Z80EX_CONTEXT *, uint16_t, uint8_t, void *) {}

static uint8_t intread_cb(Z80EX_CONTEXT *, void *) { return 0; }

dbg::dbg()
    : cpu_(nullptr), memory_(0x10000, 0), breakpoints_(), event_seq_(1), launched_(false)
{
    cpu_ = z80ex_create(
        memread_cb, this,
        memwrite_cb, this,
        portread_cb, this,
        portwrite_cb, this,
        intread_cb, this);
}

dbg::~dbg()
{
    if (cpu_)
        z80ex_destroy(cpu_);
}