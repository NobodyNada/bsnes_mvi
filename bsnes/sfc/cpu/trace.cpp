auto CPU::trace() {
  if (trace_ctx.callback) {
    if (trace_started) {
      struct trace_buffer_entry entry;

      size_t memory_effect_size  = memory_effect_buffer.size() * sizeof (struct retro_trace_memory_effect_t);
      size_t total_size = sizeof entry + memory_effect_size + fetch_buffer.size();

      uint64_t cycle = vcounter()*hperiod() + hcounter();

      entry = {
        .header = {
          .elem_size = total_size,
          .num_memory_effects = memory_effect_buffer.size(),
          .cycle = cycle
        },
        .registers = r,
        .processor_status = (uint8_t)r.p,
      };
      entry.registers.pc = trace_pc;

      size_t start = trace_buffer.size();
      trace_buffer.reallocate(start + total_size);

      uint8 *ptr = trace_buffer.data() + start;
      memcpy(ptr, &entry, sizeof entry);
      ptr += sizeof entry;

      memcpy(ptr, memory_effect_buffer.data(), memory_effect_size);
      ptr += memory_effect_size;

      memcpy(ptr, fetch_buffer.data(), fetch_buffer.size());

      memory_effect_buffer.reallocate(0);
      fetch_buffer.reallocate(0);

      if (trace_buffer.size() > trace_buffer.capacity() * 3/4) {
        flush_trace_buffer();
      }
    } else {
      trace_started = true;
      trace_buffer.reallocate(0);
      fetch_buffer.reallocate(0);
      memory_effect_buffer.reallocate(0);
    }
    trace_pc = r.pc;
  } else {
      trace_started = false;
      trace_buffer.reallocate(0);
      fetch_buffer.reallocate(0);
      memory_effect_buffer.reallocate(0);
  }
}

void CPU::flush_trace_buffer() {
  if (trace_ctx.callback) {
    trace_ctx.callback(&trace_ctx, trace_buffer.data(), trace_buffer.size());
    trace_buffer.reallocate(0);
  }
}

auto CPU::fetch() -> uint8 {
    uint8 b = WDC65816::fetch();
    if (trace_ctx.callback) {
        fetch_buffer.append(b);
    }
    return b;
}

