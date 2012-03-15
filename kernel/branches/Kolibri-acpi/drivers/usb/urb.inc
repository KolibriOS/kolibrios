;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2007-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struc URB
{
  .fd                       dd  ?
  .bk                       dd  ?
  .dev                      dd  ?    ; pointer to associated device
  .pipe                     dd  ?    ; pipe information
  .status                   dd  ?    ; non-ISO status
  .transfer_flags           dd  ?    ; URB_SHORT_NOT_OK | ...
  .transfer_buffer          dd  ?    ; associated data buffer
  .transfer_dma             dd  ?    ; dma addr for transfer_buffer
  .transfer_buffer_length   dd  ?    ; data buffer length
  .actual_length            dd  ?    ; actual transfer length
  .setup_packet             dd  ?    ; setup packet (control only)
  .setup_dma                dd  ?    ; dma addr for setup_packet
  .start_frame              dd  ?    ; start frame (ISO)
  .number_of_packets        dd  ?    ; number of ISO packets
  .interval                 dd  ?    ; transfer interval

  .error_count              dd  ?    ; number of ISO errors
  .context                  dd  ?    ; context for completion
  .complete                 dd  ?    ; (in) completion routine
  .iso_frame_desc:
}

virtual at 0
  URB URB
end virtual


struc REQ        ;usb request
{
  .request_type             db ?
  .request                  db ?
  .value                    dw ?
  .index                    dw ?
  .length                   dw ?
}

virtual at 0
  REQ REQ
end virtual

align 4
proc usb_control_msg stdcall, dev:dword, pipe:dword, request:dword,\
                         requesttype:dword, value:dword, index:dword,\
                         data:dword, size:dword, timeout:dword

           locals
             req    REQ
           endl

        lea     eax, [req]
        mov     ecx, [request]
        mov     ebx, [requesttupe]
        mov     edx, [value]
        mov     esi, [index]
        mov     edi, [size]

        mov     [eax+REQ.request_type], bl
        mov     [eax+REQ.request], cl
        mov     [eax+REQ.value], dx
        mov     [eax+REQ.index], si
        mov     [eax+REQ.length], di

        stdcall usb_internal_control_msg, [dev], [pipe], \
                eax, [data], [size], [timeout]

        ret
endp


; returns status (negative) or length (positive)
static int usb_internal_control_msg(struct usb_device *usb_dev,
                                     unsigned int pipe,
                                     struct usb_ctrlrequest *cmd,
                                     void *data, int len, int timeout)
{
   struct urb *urb;
        int     retv;
        int     length;

   urb = usb_alloc_urb(0, GFP_NOIO);
   if (!urb)
      return -ENOMEM;
   usb_fill_control_urb(urb, usb_dev, pipe, (unsigned char *)cmd, data,
                        len, usb_api_blocking_completion, NULL);

   retv = usb_start_wait_urb(urb, timeout, &length);
   if (retv < 0)
      return retv;
   else
   return length;
}


;void usb_fill_control_urb (struct urb *urb,
;                           struct usb_device *dev,
;                           unsigned int pipe,
;                           unsigned char *setup_packet,
;                           void *transfer_buffer,
;                           int buffer_length, 
;                           usb_complete_t complete_fn,
;                           void *context)
;{
;
;   urb->dev = dev;
;   urb->pipe = pipe;
;   urb->setup_packet = setup_packet;
;   urb->transfer_buffer = transfer_buffer;
;   urb->transfer_buffer_length = buffer_length;
;   urb->complete = complete_fn;
;   urb->context = context;
;}
;
