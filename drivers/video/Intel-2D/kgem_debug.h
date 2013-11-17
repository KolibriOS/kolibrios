#ifndef KGEM_DEBUG_H
#define KGEM_DEBUG_H

void
kgem_debug_print(const uint32_t *data,
		 uint32_t offset, unsigned int index,
		 const char *fmt, ...);

struct drm_i915_gem_relocation_entry *
kgem_debug_get_reloc_entry(struct kgem *kgem, uint32_t offset);

struct kgem_bo *
kgem_debug_get_bo_for_reloc_entry(struct kgem *kgem,
				  struct drm_i915_gem_relocation_entry *reloc);

int kgem_gen7_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen7_finish_state(struct kgem *kgem);

int kgem_gen6_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen6_finish_state(struct kgem *kgem);

int kgem_gen5_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen5_finish_state(struct kgem *kgem);

int kgem_gen4_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen4_finish_state(struct kgem *kgem);

int kgem_gen3_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen3_finish_state(struct kgem *kgem);

int kgem_gen2_decode_3d(struct kgem *kgem, uint32_t offset);
void kgem_gen2_finish_state(struct kgem *kgem);

#endif
