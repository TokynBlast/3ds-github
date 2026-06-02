use ctru::{prelude::*, services::gfx::{Flush, Swap, Screen}};

fn main() {
    let apt = Apt::new().unwrap();
    let gfx = Gfx::new().unwrap();
    let mut top_screen = gfx.top_screen.borrow_mut();
    let image_bytes = include_bytes!("../miku.raw");

    while apt.main_loop() {
        let buffer = top_screen.raw_framebuffer();
        unsafe {
            std::ptr::copy_nonoverlapping(
              image_bytes.as_ptr(),
                buffer.ptr,
                image_bytes.len(),
            );
        }

        top_screen.flush_buffers();
        top_screen.swap_buffers();

        gfx.wait_for_vblank();
    }
}
