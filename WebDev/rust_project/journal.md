# 4/25
- https://www.youtube.com/watch?v=L8tWKqSMKUI&list=PL2q9pua8FpiUwVll2Alk8W_d3eN8IxJjO&ab_channel=CodetotheMoon
- https://actix.rs/docs/application
Just doing initial project setup and playing around with the start documentation
- example of mongodb with actix: https://github.com/actix/examples/tree/master/databases/mongodb

# 4/27
Spent like 1.5 hours getting the db connection setup. Still havn't fully tested it but its almost 10:30 and stuff compiled. Tried to follow
https://github.com/actix/examples/blob/master/databases/mongodb/src/main.rs but took a long ass time to modify it to my own stuff. Spent a lot
of time fighting with the type system. GPT was of little utility because of how niche the framework + db driver is. I've been kind of copying code
at a high level so there are some conecepts/documenation I need to look into...
- the doc! maco
- serde package
- future package

# 5/2
Found a guide for integrating auth0 with actix: https://auth0.com/blog/build-an-api-in-rust-with-jwt-authentication-using-actix-web/
Played around with the source code and looked at more technical documentation: https://docs.rs/mongodb/2.0.0/mongodb/struct.Cursor.html, https://www.mongodb.com/docs/drivers/rust/current/fundamentals/crud/read-operations/cursor/. Managed to get DB reads working.

Via a little GPT and checking the actix-web docs I got path extraction and request body extraction working. Also, the autocomplete descriptions from the rust LSP help a ton when
trying to work with the type system.

# 5/21
Spent like 5 hours marathoning the project to near completion. Everything works as intended. Just need to add in auth0. Things take so much longer to write in Rust. This is mostly
because I'm fighting with the type system. However I like Rusts functional features a lot. It makes dealing with return values really satisfying. As far as learning a new framework the concepts feel nearly 1:1 from Node/React. I havn't struggled with them at a high level at all. There are some nuances with the framework that are introduced from the framework being in rust that have tripped me up. This mainly has to do with functions returning types that are more complicated (encapsulates in Option<>/Result<>) than I'm expecting. I also tried really hard to serve static files from a directory and kept hitting 404s. The work around was to just serve each file individiually. I may be doing something wrong but googling it hasn't yielded any results so I'm pretty stumpted. This is definetly one of the major drawbacks of using a niche frameowork like actix.
