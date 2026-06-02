use ctru::{prelude::*, services::gfx::{Flush, Swap, Screen}};

fn main() {
    let apt = Apt::new().unwrap();
    let gfx = Gfx::new().unwrap();
    let mut top_screen = gfx.top_screen.borrow_mut();
    let mut bottom_screen = gfx.bottom_screen.borrow_mut();
    let gh_icon = include_bytes!("assets/icon.raw");

    while apt.main_loop() {
        let top_buff = top_screen.raw_framebuffer();
        let bottom_buff = bottom_screen.raw_framebuffer();

        // Calculate exact byte capacities
        let top_bytes = (top_buff.width * bottom_buff.height * 3) as usize;       // 288,000 bytes
        let bottom_bytes = (bottom_buff.width * bottom_buff.height * 3) as usize; // 230,400 bytes

        // 3DS works with BGR, not RGB (this is stupid ;-;)
        unsafe {
            // #0F61A5 Background on top screen
            let mut i = 0;
            while i < top_bytes {
                *top_buff.ptr.add(i)     = 0xA5; // Blue
                *top_buff.ptr.add(i + 1) = 0x61; // Green
                *top_buff.ptr.add(i + 2) = 0x0F; // Red
                i += 3;
            }

            // #0F61A5 Background on bottom screen
            let mut j = 0;
            while j < bottom_bytes {
                *bottom_buff.ptr.add(j)     = 0xA5; // Blue
                *bottom_buff.ptr.add(j + 1) = 0x61; // Green
                *bottom_buff.ptr.add(j + 2) = 0x0F; // Red
                j += 3;
            }

            // GitHub logo (top screen)
            std::ptr::copy_nonoverlapping(
                gh_icon.as_ptr(),
                top_buff.ptr,
                gh_icon.len(),
            );
        }

        top_screen.flush_buffers();
        top_screen.swap_buffers();
        bottom_screen.flush_buffers();
        bottom_screen.swap_buffers();

        gfx.wait_for_vblank();
    }
}
