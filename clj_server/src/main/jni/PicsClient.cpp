#include "com_pics_PicsClient.h"

#include <vector>
#include <string>
#include <iostream>

#include "Client.hpp"

JNIEXPORT jlong JNICALL Java_com_pics_PicsClient__1connect
  (JNIEnv *, jobject) {
  try {
  	return reinterpret_cast<jlong>(new Client);
  } catch(...) {}

  return 0;
}

JNIEXPORT void JNICALL Java_com_pics_PicsClient__1disconnect
  (JNIEnv *, jobject, jlong client) {
	delete reinterpret_cast<Client *>(client);
}

JNIEXPORT jboolean JNICALL Java_com_pics_PicsClient__1isSearching
  (JNIEnv *, jobject, jlong client) {
  auto c = reinterpret_cast<Client *>(client);
  return c->isSearching();
}

JNIEXPORT jobjectArray JNICALL Java_com_pics_PicsClient__1search
  (JNIEnv *env, jobject, jlong client, jobjectArray desc) {

  auto c = reinterpret_cast<Client *>(client);
	auto count = env->GetArrayLength(desc);

	std::vector<std::string> searchDesc;
	searchDesc.reserve(count);

	for (auto i = 0; i != count; ++i) {
		auto bytes = (jbyteArray)(env->GetObjectArrayElement(desc, i));
		auto bytesCount = env->GetArrayLength(bytes);

		std::string eachDesc(bytesCount, '\0');
		env->GetByteArrayRegion(bytes, 0, bytesCount, reinterpret_cast<jbyte *>(&eachDesc.operator[](0)));
		
		searchDesc.push_back(std::move(eachDesc));
	}

	// do searching
  auto result = std::get<0>(c->search(searchDesc));

  auto resultArray = (jobjectArray)env->NewObjectArray(result.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
  
  auto resultCounter = 0;
  for (const auto& e : result) {
  	env->SetObjectArrayElement(resultArray, resultCounter, env->NewStringUTF(e.c_str()));
  	++resultCounter;
  }

  return resultArray;
}