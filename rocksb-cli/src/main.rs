use rocksdb::{Options, DB};
use flate2::read::GzDecoder;
use std::io::Read;
use clap::{Arg, App, SubCommand};

fn main() {
    let matches = App::new("RocksDB CLI")
        .arg(Arg::with_name("path")
            .help("Path to the RocksDB")
            .required(true)
            .index(1))
        .subcommand(SubCommand::with_name("get")
            .about("Get the value of a key from the database")
            .arg(Arg::with_name("key")
                .help("Key to get the value for")
                .required(true)
                .index(1)))
        .subcommand(SubCommand::with_name("delete")
            .about("Delete a key from the database")
            .arg(Arg::with_name("key")
                .help("Key to delete")
                .required(true)
                .index(1)))
        .subcommand(SubCommand::with_name("list")
            .about("List all the keys in the database"))
        .subcommand(SubCommand::with_name("list+decompress")
            .about("List a specified number of keys in the database, with optional jump parameter, and decompress the values using flate2")
            .arg(Arg::with_name("count")
                .help("Number of keys to list")
                .required(true)
                .index(1))
            .arg(Arg::with_name("jump")
                .help("Optional jump parameter")
                .required(false)
                .index(2)))
        .get_matches();

    let path = matches.value_of("path").unwrap();
    let mut options = Options::default();
    options.create_if_missing(true);
    let db = DB::open(&options, path).unwrap();

    if let Some(matches) = matches.subcommand_matches("get") {
        let key = matches.value_of("key").unwrap();
        match db.get(key.as_bytes()) {
            Ok(Some(value)) => println!("Value: {:?}", String::from_utf8(value).unwrap()),
            Ok(None) => println!("Key not found"),
            Err(e) => println!("Error: {}", e),
        }
    } else if let Some(matches) = matches.subcommand_matches("delete") {
        let key = matches.value_of("key").unwrap();
        match db.delete(key.as_bytes()) {
            Ok(_) => println!("Key deleted"),
            Err(e) => println!("Error: {}", e),
        }
    } else if let Some(_) = matches.subcommand_matches("list") {
        let iter = db.iterator(rocksdb::IteratorMode::Start);
        for item in iter {
            match item {
                Ok((key, _)) => println!("Key: {:?}", String::from_utf8(key.to_vec()).unwrap()),
                Err(e) => println!("Error: {}", e),
            }
        }
    } else if let Some(matches) = matches.subcommand_matches("list+decompress") {
        let count = matches.value_of("count").unwrap().parse::<usize>().unwrap();
        let jump = matches.value_of("jump").unwrap_or("1").parse::<usize>().unwrap();
        let mut iter = db.iterator(rocksdb::IteratorMode::Start);
        let mut counter = 0;

        while let Some(item) = iter.next() {
            match item {
                Ok((key, value)) => {
                    if counter % jump == 0 {
                        let mut d = GzDecoder::new(&value[..]);
                        let mut decompressed = String::new();
                        d.read_to_string(&mut decompressed).unwrap();
                        println!("Key: {:?}, Value: {:?}", String::from_utf8(key.to_vec()).unwrap(), decompressed);
                    }

                    counter += 1;
                    if counter >= count {
                        break;
                    }
                }
                Err(e) => println!("Error: {}", e),
            }
        }
    }
}