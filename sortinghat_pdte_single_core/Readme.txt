
cd sortinghat_pdte_single_core

cargo build --release
cargo run --release ./data/heart_11bits 10
cargo run --release ./data/breast_11bits 10
cargo run --release ./data/spam_11bits 10
cargo run --release ./data/electricity_10bits 10
cargo run --release ./data/steel_11bits 10