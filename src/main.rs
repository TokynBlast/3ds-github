use ctru::prelude::*;
use ctru::services::gfx::{Flush, Screen, Swap};
use ctru::services::hid::KeyPad;

fn main() {
    let apt = Apt::new().unwrap();
    let mut hid = Hid::new().unwrap();
    let gfx = Gfx::new().unwrap();

    let mut top_screen = gfx.top_screen.borrow_mut();
    let image_bytes = include_bytes!("../miku.raw");

    while apt.main_loop() {
        hid.scan_input();

        if hid.keys_down().contains(KeyPad::START) {
            break;
        }

        let buffer = top_screen.raw_framebuffer();

        // 2. Safely calculate total screen bytes based on dimensions and format
        // (For standard 24-bit BGR8/RGB8, this is width * height * 3)
        let total_pixels = (buffer.width * buffer.height) as usize;
        let buffer_size = total_pixels * 3;

        // 3. Direct memory copy to the top-left buffer pointer
        unsafe {
            std::ptr::copy_nonoverlapping(
                image_bytes.as_ptr(),
                buffer.ptr,
                image_bytes.len().min(buffer_size),
            );
        }

        top_screen.flush_buffers();
        top_screen.swap_buffers();

        gfx.wait_for_vblank();
    }
}
