// src/middleware.rs
use actix_web::{dev::Service, dev::ServiceRequest, Error, HttpMessage};
use futures::future::{ok, Ready};
use futures::FutureExt;

use crate::jwt::validate_jwt;

pub struct AuthMiddleware<S> {
    service: S,
}

impl<S, B> actix_service::Transform<S, ServiceRequest> for AuthMiddleware<S>
where
    S: Service<ServiceRequest, Response = actix_web::dev::ServiceResponse<B>, Error = Error>
        + 'static,
    S::Future: 'static,
    B: 'static,
{
    type Response = S::Response;
    type Error = S::Error;
    type Transform = AuthMiddlewareService<S>;
    type InitError = ();
    type Future = Ready<Result<Self::Transform, Self::InitError>>;

    fn new_transform(&self, service: S) -> Self::Future {
        ok(AuthMiddlewareService { service })
    }
}

pub struct AuthMiddlewareService<S> {
    service: S,
}

impl<S, B> Service<ServiceRequest> for AuthMiddlewareService<S>
where
    S: Service<ServiceRequest, Response = actix_web::dev::ServiceResponse<B>, Error = Error>
        + 'static,
    S::Future: 'static,
    B: 'static,
{
    type Response = S::Response;
    type Error = S::Error;
    type Future = futures::future::Either<S::Future, Ready<Result<Self::Response, Self::Error>>>;

    fn poll_ready(
        &self,
        cx: &mut std::task::Context<'_>,
    ) -> std::task::Poll<Result<(), Self::Error>> {
        self.service.poll_ready(cx)
    }

    fn call(&self, req: ServiceRequest) -> Self::Future {
        if let Some(auth_header) = req.headers().get("Authorization") {
            if let Ok(auth_str) = auth_header.to_str() {
                if auth_str.starts_with("Bearer ") {
                    let token = &auth_str[7..];
                    if validate_jwt(token) {
                        return futures::future::Either::Left(self.service.call(req));
                    }
                }
            }
        }

        futures::future::Either::Right(ok(
            req.into_response(actix_web::HttpResponse::Unauthorized().finish().into_body())
        ))
    }
}
