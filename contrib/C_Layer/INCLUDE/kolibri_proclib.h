#ifndef KOLIBRI_PROCLIB_H
#define KOLIBRI_PROCLIB_H

/// @brief Initialize proclib
/// @return -1 if unsucessful
extern int kolibri_proclib_init(void);

enum Mode {
	OD_OPEN,
	OD_SAVE,
	OD_DIR
};

#endif /* KOLIBRI_PROCLIB_H */
