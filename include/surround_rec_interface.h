/*
 * Extrapolated / reversed header for SSR
 */

#ifndef _SURROUND_REC_INTERFACE_H_
#define _SURROUND_REC_INTERFACE_H_

typedef char *get_param(void *arg);
typedef void set_param(void *arg, const char *arg1);

typedef struct get_param_data {
    const char *name;
    get_param *get_param_fn;
} get_param_data_t;

typedef struct set_param_data {
    const char *name;
    set_param *set_param_fn;
} set_param_data_t;

const get_param_data_t* surround_rec_get_get_param_data(void);

const set_param_data_t* surround_rec_get_set_param_data(void);

int surround_rec_init(void **arg, int arg1, int arg2, int arg3,
                      int arg4, const char *arg5);

void surround_rec_deinit(void *arg);

void surround_rec_process(void *arg, const int16_t *arg1, int16_t *arg2);

#endif /* #ifndef _SURROUND_REC_INTERFACE_H_ */
