(ns pics.core
  (:require [ring.adapter.jetty :as jetty]
            [compojure.core :refer :all]
            [compojure.handler :as handler]
            [ring.middleware.json :as json]
            [schema.core :as s]
            [clojure.tools.logging :refer [info]])
  (:import [com.pics PicsClient ServiceNotAvailableException]
           [java.util Base64])

  (:gen-class))

(def client (atom nil))

(s/defschema SearchDescriptors {:alg (s/enum "ORB")
                                :descriptors [s/Str]})

(defn- bad-args
  [msg]
  {:status 400 :body {:message msg}})

(defn- reply
  [result error]
  {:status 200 :body {:result (seq result)
                      :error error}})

(defn- get-search-result
  [descriptors]
  (let [bytes (to-array descriptors)]
    (.search @client bytes)))

(defn- search [{body :body}]
  (s/validate SearchDescriptors body)

  (let [descriptors (map #(.decode (Base64/getDecoder) %) (:descriptors body))]
    ;; ORB descriptor has fix 32-bytes size
    (if (every? #(= (count %) 32) descriptors)
      (try 
        (reply (get-search-result descriptors) nil)
        (catch ServiceNotAvailableException e
          (reply nil "Busy")))

      (bad-args "Invalid descriptors"))))

(defroutes app
  (POST "/search" req (search req)))

(def handlers
  (-> (handler/site app)
      json/wrap-json-response
      (json/wrap-json-body {:keywords? true})))

(defn -main
  [& args]
  (reset! client (new PicsClient))
  (.connect @client)

  (jetty/run-jetty handlers {:port 40001})
  (.disconnect @client))