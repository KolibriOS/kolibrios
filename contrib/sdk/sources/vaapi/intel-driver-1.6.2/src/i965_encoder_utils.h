#ifndef __I965_ENCODER_UTILS_H__
#define __I965_ENCODER_UTILS_H__

int 
build_avc_slice_header(VAEncSequenceParameterBufferH264 *sps_param, 
                       VAEncPictureParameterBufferH264 *pic_param,
                       VAEncSliceParameterBufferH264 *slice_param,
                       unsigned char **slice_header_buffer);
int 
build_avc_sei_buffering_period(int cpb_removal_length,
                               unsigned int init_cpb_removal_delay, 
                               unsigned int init_cpb_removal_delay_offset,
                               unsigned char **sei_buffer);

int
build_avc_sei_pic_timing(unsigned int cpb_removal_length, unsigned int cpb_removal_delay, 
                         unsigned int dpb_output_length, unsigned int dpb_output_delay,
                         unsigned char **sei_buffer);

int 
build_avc_sei_buffer_timing(unsigned int init_cpb_removal_length,
				unsigned int init_cpb_removal_delay,
				unsigned int init_cpb_removal_delay_offset,
				unsigned int cpb_removal_length,
				unsigned int cpb_removal_delay,
				unsigned int dpb_output_length,
				unsigned int dpb_output_delay,
				unsigned char **sei_buffer);

int 
build_mpeg2_slice_header(VAEncSequenceParameterBufferMPEG2 *sps_param,
                         VAEncPictureParameterBufferMPEG2 *pic_param,
                         VAEncSliceParameterBufferMPEG2 *slice_param,
                         unsigned char **slice_header_buffer);

/* HEVC */

int
build_hevc_slice_header(VAEncSequenceParameterBufferHEVC *seq_param,
                        VAEncPictureParameterBufferHEVC *pic_param,
                        VAEncSliceParameterBufferHEVC *slice_param,
                        unsigned char **header_buffer,
                        int slice_index);
int
build_hevc_sei_buffering_period(int cpb_removal_length,
                                unsigned int init_cpb_removal_delay,
                                unsigned int init_cpb_removal_delay_offset,
                                unsigned char **sei_buffer);

int
build_hevc_sei_pic_timing(unsigned int cpb_removal_length, unsigned int cpb_removal_delay,
                          unsigned int dpb_output_length, unsigned int dpb_output_delay,
                          unsigned char **sei_buffer);

int
build_hevc_idr_sei_buffer_timing(unsigned int init_cpb_removal_delay_length,
                                 unsigned int init_cpb_removal_delay,
                                 unsigned int init_cpb_removal_delay_offset,
                                 unsigned int cpb_removal_length,
                                 unsigned int cpb_removal_delay,
                                 unsigned int dpb_output_length,
                                 unsigned int dpb_output_delay,
                                 unsigned char **sei_buffer);

#endif /* __I965_ENCODER_UTILS_H__ */
