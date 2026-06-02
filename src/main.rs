use ctru;
use ctru::console::Console;
use ctru::services::gfx::Gfx;

fn main() {
    // Initialize graphics (using framebuffers allocated on the HEAP).
    let gfx = Gfx::new().unwrap();

    // Create a `Console` that takes control of the upper LCD screen.
    let _ = Console::new(gfx.top_screen.borrow_mut());

    println!("This is a test, development for 3DS Git will begin!!");

    loop {}
}
