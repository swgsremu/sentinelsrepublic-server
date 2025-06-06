//
// Created by Victor Popovici on 7/2/17.
//

#include "CloseObjectsVector.h"

#include "server/zone/TreeEntry.h"

CloseObjectsVector::CloseObjectsVector() : messageReceivers() {
	objects.setNoDuplicateInsertPlan();

	messageReceivers.setNoDuplicateInsertPlan();

	count = 0;
}

void CloseObjectsVector::safeCopyTo(Vector<ManagedReference<TreeEntry*> >& vec) const {
	vec.removeAll(size(), size() / 2);

	ReadLocker locker(&mutex);

	//vec.addAll(*this);
	for (int i = 0; i < objects.size(); ++i) {
		const auto& obj = objects.getUnsafe(i);

		vec.emplace(obj);
	}
}

SortedVector<ManagedReference<TreeEntry*> > CloseObjectsVector::getSafeCopy() const {
	ReadLocker locker(&mutex);

	SortedVector<ManagedReference<TreeEntry*> > copy;

	for (int i = 0; i < objects.size(); ++i) {
		const auto& obj = objects.getUnsafe(i);

		copy.emplace(obj.get());
	}

	return copy;
}

void CloseObjectsVector::safeCopyTo(Vector<TreeEntry*>& vec) const {
	vec.removeAll(size(), size() / 2);

	ReadLocker locker(&mutex);

	for (int i = 0; i < objects.size(); ++i) {
		vec.add(objects.getUnsafe(i).get());
	}
}

bool CloseObjectsVector::contains(const Reference<TreeEntry*>& o) const {
	ReadLocker locker(&mutex);

	bool ret = objects.find(o) != -1;

	return ret;
}

void CloseObjectsVector::removeAll(int newSize, int newIncrement) {
	Locker locker(&mutex);

	objects.removeAll(newSize, newIncrement);

	messageReceivers.removeAll(newSize, newIncrement);

	count = 0;
}

void CloseObjectsVector::dropReceiver(TreeEntry* entry) {
	uint32 receiverTypes = entry->registerToCloseObjectsReceivers();

	if (receiverTypes && messageReceivers.size()) {
		for (int i = 0; i < CloseObjectsVector::Types::SIZE; ++i) {
			uint32 type = 1 << i;

			if (receiverTypes & type) {
				int idx = messageReceivers.find(type);

				if (idx != -1) {
					auto& receivers = messageReceivers.elementAt(idx).getValue();

					receivers.drop(entry);
				}
			}
		}
	}
}

Reference<TreeEntry*> CloseObjectsVector::remove(int index) {
	Locker locker(&mutex);

	const auto& ref = objects.get(index);

	dropReceiver(ref);

	auto obj = objects.remove(index);

	count = objects.size();

	return obj;
}

bool CloseObjectsVector::drop(const Reference<TreeEntry*>& o) {
	Locker locker(&mutex);

	dropReceiver(o);

	auto res = objects.drop(o);

	count = objects.size();

	return res;
}

void CloseObjectsVector::safeCopyReceiversTo(Vector<TreeEntry*>& vec, uint32 receiverType) const {
	ReadLocker locker(&mutex);

	int i = messageReceivers.find(receiverType);

	if (i != -1) {
		const auto& receivers = messageReceivers.elementAt(i).getValue();

		vec.removeAll(receivers.size(), receivers.size() / 2);

		vec.addAll(receivers);
	}
}

void CloseObjectsVector::safeRunForEach(const Function<void(TreeEntry* const&)>& lambda, uint32 receiverType) const {
	ReadLocker locker(&mutex);

	int i = messageReceivers.find(receiverType);

	if (i != -1) {
		const auto& receivers = messageReceivers.elementAt(i).getValue();

		receivers.forEach(lambda);
	}
}

void CloseObjectsVector::safeCopyReceiversTo(Vector<ManagedReference<TreeEntry*> >& vec, uint32 receiverType) const {
	ReadLocker locker(&mutex);

	int i = messageReceivers.find(receiverType);

	if (i != -1) {
		const auto& receivers = messageReceivers.elementAt(i).getValue();

		vec.removeAll(receivers.size(), receivers.size() / 2);

		for (int i = 0; i < receivers.size(); ++i)
			vec.emplace(receivers.getUnsafe(i));
	}
}

void CloseObjectsVector::safeAppendReceiversTo(Vector<TreeEntry*>& vec, uint32 receiverType) const {
	ReadLocker locker(&mutex);

	int i = messageReceivers.find(receiverType);

	if (i != -1) {
		const auto& receivers = messageReceivers.elementAt(i).getValue();
		vec.addAll(receivers);
	}
}

void CloseObjectsVector::safeAppendReceiversTo(Vector<ManagedReference<TreeEntry*> >& vec, uint32 receiverType) const {
	ReadLocker locker(&mutex);

	int i = messageReceivers.find(receiverType);

	if (i != -1) {
		const auto& receivers = messageReceivers.elementAt(i).getValue();
		for (int i = 0; i < receivers.size(); ++i)
			vec.emplace(receivers.getUnsafe(i));
	}
}

const Reference<TreeEntry*>& CloseObjectsVector::get(int idx) const {
	return objects.get(idx);
}

void CloseObjectsVector::putReceiver(TreeEntry* entry, uint32 receiverTypes) {
	if (receiverTypes) {
		for (int i = 0; i < CloseObjectsVector::Types::SIZE; ++i) {
			uint32 type = 1 << i;

			if (receiverTypes & type) {
				int idx = messageReceivers.find(type);

				if (idx != -1) {
					auto& receivers = messageReceivers.elementAt(idx).getValue();

					receivers.put(entry);
				} else {
					SortedVector<TreeEntry*> vec;
					vec.setNoDuplicateInsertPlan();

					vec.put(entry);

					messageReceivers.put(std::move(type), std::move(vec));
				}
			}
		}
	}
}

int CloseObjectsVector::put(const Reference<TreeEntry*>& o) {
	uint32 receiverTypes = o->registerToCloseObjectsReceivers();

	Locker locker(&mutex);

	putReceiver(o.get(), receiverTypes);

	auto res = objects.put(o);

	count = objects.size();

	return res;
}

int CloseObjectsVector::put(Reference<TreeEntry*>&& o) {
	uint32 receiverTypes = o->registerToCloseObjectsReceivers();

	Locker locker(&mutex);
	putReceiver(o.get(), receiverTypes);

	auto res = objects.put(std::move(o));

	count = objects.size();

	return res;
}
