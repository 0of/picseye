(ns pics-fetch.core
  (:require [clojure.data.csv :as csv]  
            [clojure.java.io :as io]   
            [clj-http.client :as client])
  (:gen-class))

(defn read-pictures
  []
  (with-open [in-file (io/reader (io/resource ""))]
    (doall (next (csv/read-csv in-file)))))

(defn cdn-prefix
  [suffix]
  (if (re-find #"^http" suffix)
    suffix
    (str "" suffix)))

(defn- download
  [url file-location]
  (io/copy         
    (:body (client/get url {:as :stream}))
    (io/file file-location)))

(defn download-picts
  [pict-pairs]
  (letfn [(download-each [i [suffix art-id]]
           (let [file-location (format "/tmp/download/%d.jpg" i)
                 pict-url (cdn-prefix suffix)]
              (try 
                (download pict-url file-location)
                (Thread/sleep 200)
                [file-location art-id]

                (catch Exception e
                  ["error" (.getMessage e)]))))]

    (keep-indexed download-each pict-pairs)))

(defn log-downloaded
 [entries] 
 (with-open [out-file (io/writer "download-log-1.csv")]
   (csv/write-csv out-file entries)))

(defn -main
  [& args]
  (-> (read-pictures)
      download-picts
      log-downloaded))
