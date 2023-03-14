provider "kubernetes" {
  config_path = "~/.kube/config"
}

resource "kubernetes_deployment" "mongodb" {
  metadata {
    name = "mongodb-deployment"
    labels = {
      app = "mongodb"
    }
  }
  spec {
    replicas = 1
    selector {
      match_labels = {
        app = "mongodb"
      }
    }
    template {
      metadata {
        labels = {
          app = "mongodb"
        }
      }
      spec {
        container {
          name  = "mongodb"
          image = "mongo"
          port {
            container_port = 27017
          }
          env {
            name = "MONGO_INITDB_ROOT_USERNAME"
            value_from {
              secret_key_ref {
                name = "mongodb-secret"
                key  = "mongo-root-username"
              }
            }
          }
          env {
            name = "MONGO_INITDB_ROOT_PASSWORD"
            value_from {
              secret_key_ref {
                name = "mongodb-secret"
                key  = "mongo-root-password"
              }
            }
          }
        }
      }
    }
  }
}
resource "kubernetes_service" "mongodb" {
  metadata {
    name = "mongodb-service"
  }
  spec {
    selector = {
      app = "mongodb"
    }
    port {
      protocol    = "TCP"
      port        = 27017
      target_port = 27017
    }
  }
}

resource "kubernetes_ingress" "app" {
  metadata {
    name = "name"
    annotations = {
      "kubernetes.io/ingress.class" = "nginx"
    }
  }

  spec {
    rule {
      host = "app.com"

      http {
        path {
          backend {
            service_name = "my-service"
            service_port = 8080
          }
        }
      }
    }
  }
}

resource "kubernetes_config_map" "mongodb" {
  metadata {
    name = "mongodb-configmap"
  }

  data = {
    database_url = "mongodb-service"
  }
}

resource "kubernetes_deployment" "mongo_express" {
  metadata {
    name = "mongo-express"
    labels = {
      app = "mongo-express"
    }
  }

  spec {
    replicas = 1

    selector {
      match_labels = {
        app = "mongo-express"
      }
    }

    template {
      metadata {
        labels = {
          app = "mongo-express"
        }
      }

      spec {
        container {
          name  = "mongo-express"
          image = "mongo-express"

          port {
            container_port = 8081
          }

          env {
            name = "ME_CONFIG_MONGODB_ADMINUSERNAME"
            value_from {
              secret_key_ref {
                name = "mongodb-secret"
                key  = "mongo-root-username"
              }
            }
          }

          env {
            name = "ME_CONFIG_MONGODB_ADMINPASSWORD"
            value_from {
              secret_key_ref {
                name = "mongodb-secret"
                key  = "mongo-root-password"
              }
            }
          }

          env {
            name = "ME_CONFIG_MONGODB_SERVER"
            value_from {
              config_map_key_ref {
                name = "mongodb-configmap"
                key  = "database_url"
              }
            }
          }
        }
      }
    }
  }
}

resource "kubernetes_service" "mongo_express" {
  metadata {
    name = "mongo-express-service"
  }

  spec {
    selector = {
      app = "mongo-express"
    }

    type = "LoadBalancer"

    port {
      protocol    = "TCP"
      port        = 8081
      target_port = 8081
      node_port   = 30000
    }
  }
}

resource "kubernetes_secret" "mongodb" {
  metadata {
    name = "mongodb-secret"
  }

  type = "Opaque"

  data = {
    mongo-root-username = "dXNlcm5hbWU="
    mongo-root-password = "cGFzc3dvcmQ="
  }
}