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
