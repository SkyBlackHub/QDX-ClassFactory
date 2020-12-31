#ifndef QDX_METACLASSMANAGER_H
#define QDX_METACLASSMANAGER_H

#include <QVariantHash>
#include <QSet>

namespace QDX {

	template <typename C>
	class ClassFactory;

	template <typename C>
	class ClassRecord
	{
	public:
		ClassRecord(const QString &name, ClassFactory<C> *factory = nullptr);
		ClassRecord(const QString &name, const QVariantHash &data, ClassFactory<C> *factory = nullptr);

		virtual ~ClassRecord();

		ClassFactory<C> *factory() const;
		void setFactory(ClassFactory<C> *factory);

		bool bind(const QString &alias);

		QString name() const;

		C *instance() const;
		void destroy(C *object) const;

		const QSet<const C *> &instances() const;
		void destroyAll();

		QVariantHash data() const;
		void setData(const QVariantHash &data);

		bool isRegistrationEnabled() const;
		void setRegistrationEnabled(bool registration_enabled);

	protected:
		virtual C *createObject() const = 0;
		virtual void deleteObject(C *object) const = 0;

		QString m_name;
		QVariantHash m_data;
		ClassFactory<C> *m_factory = nullptr;

		bool m_registration_enabled = false;
		mutable QSet<const C *> m_instances;
	};

	template <typename T, typename C>
	class ClassMimicRecord : public ClassRecord<C>
	{
	public:
		using ClassRecord<C>::ClassRecord;

	protected:
		virtual C *createObject() const override;
		virtual void deleteObject(C *object) const override;
	};

	template <typename C>
	class ClassFactory
	{
	public:
		~ClassFactory();

		template <typename T>
		QDX::ClassRecord<C> *addClass(const QString &name);
		template <typename T>
		QDX::ClassRecord<C> *addClass(const QString &name, const QVariantHash &data);

		QDX::ClassRecord<C> *record(const QString &name) const;

		void addRecord(QDX::ClassRecord<C> *record);

		QDX::ClassRecord<C> *takeRecord(QDX::ClassRecord<C> *record);
		QDX::ClassRecord<C> *takeRecord(const QString &name);

		void removeRecord(QDX::ClassRecord<C> *record);
		void removeRecord(const QString &name);

		bool contains(const QString &name) const;
		bool contains(QDX::ClassRecord<C> *record) const;

		C *instance(const QString &name) const;
		void destroy(const QString &name, C *object) const;
		void destroy(C *object) const;

		bool bind(const QString &alias, QDX::ClassRecord<C> *record);
		bool bind(const QString &alias, const QString &name);
		void unbind(const QString &name);

		bool isRegistrationEnabled() const;
		void setRegistration(bool registration);

		QDX::ClassRecord<C> *findRecord(const C *object) const;
		QList<QDX::ClassRecord<C> *> records() const;

	private:
		QSet<QDX::ClassRecord<C> *> m_records;
		QHash<QString, QDX::ClassRecord<C> *> m_aliases;
		bool m_registration_enabled = false;
	};

} // namespace QDX

template<typename C>
QDX::ClassRecord<C>::ClassRecord(const QString &name, ClassFactory<C> *factory) : m_name(name), m_factory(factory)
{
	if (m_factory) {
		m_factory->addRecord(this);
	}
}

template<typename C>
QDX::ClassRecord<C>::ClassRecord(const QString &name, const QVariantHash &data, ClassFactory<C> *factory) : m_name(name), m_data(data), m_factory(factory)
{
	if (m_factory) {
		m_factory->addRecord(this);
	}
}

template <typename C>
QDX::ClassRecord<C>::~ClassRecord()
{
	if (m_factory) {
		m_factory->takeRecord(this);
	}
}

template<typename C>
QDX::ClassFactory<C> *QDX::ClassRecord<C>::factory() const
{
	return m_factory;
}

template <typename C>
void QDX::ClassRecord<C>::setFactory(ClassFactory<C> *factory)
{
	if (m_factory == factory) {
		return;
	}
	if (m_factory) {
		m_factory->takeRecord(this);
	}
	m_factory = factory;
	if (factory) {
		factory->addRecord(this);
	}
}

template<typename C>
bool QDX::ClassRecord<C>::bind(const QString &alias)
{
	return m_factory ? m_factory->bind(alias, this) : false;
}

template<typename C>
const QSet<const C *> &QDX::ClassRecord<C>::instances() const
{
	return m_instances;
}

template<typename C>
void QDX::ClassRecord<C>::destroyAll()
{
	for (const C *object : m_instances) {
		this->deleteObject(object);
	}
}

template<typename C>
QString QDX::ClassRecord<C>::name() const
{
	return m_name;
}

template<typename C>
C *QDX::ClassRecord<C>::instance() const
{
	if (m_registration_enabled) {
		C* instance = this->createObject();
		m_instances.insert(instance);
		return instance;
	}
	return this->createObject();
}

template<typename C>
void QDX::ClassRecord<C>::destroy(C *object) const
{
	if (m_registration_enabled) {
		m_instances.remove(object);
	}
	this->deleteObject(object);
}

template <typename C>
QVariantHash QDX::ClassRecord<C>::data() const
{
	return m_data;
}

template <typename C>
void QDX::ClassRecord<C>::setData(const QVariantHash &data)
{
	m_data = data;
}

template <typename C>
bool QDX::ClassRecord<C>::isRegistrationEnabled() const
{
	return m_registration_enabled;
}

template <typename C>
void QDX::ClassRecord<C>::setRegistrationEnabled(bool registration_enabled)
{
	m_registration_enabled = registration_enabled;
}

template<typename T, typename C>
C *QDX::ClassMimicRecord<T, C>::createObject() const
{
	return new T;
}

template<typename T, typename C>
void QDX::ClassMimicRecord<T, C>::deleteObject(C *object) const
{
	delete ((T *) object);
}

template <typename C>
QDX::ClassFactory<C>::~ClassFactory()
{
	if (m_records.isDetached()) {
		QSet<QDX::ClassRecord<C> *> records = m_records;
		m_records.clear();
		for (QDX::ClassRecord<C> *record : records) {
			delete record;
		}
	}
}

template <typename C> template <typename T>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::addClass(const QString &name)
{
	QDX::ClassMimicRecord<T, C> *record = new QDX::ClassMimicRecord<T, C>(name, this);
	record->setRegistrationEnabled(m_registration_enabled);
	return record;
}

template <typename C> template <typename T>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::addClass(const QString &name, const QVariantHash &data)
{
	QDX::ClassMimicRecord<T, C> *record = new QDX::ClassMimicRecord<T, C>(name, data, this);
	record->setRegistrationEnabled(m_registration_enabled);
	return record;
}

template <typename C>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::record(const QString &name) const
{
	return m_aliases.value(name, nullptr);
}

template <typename C>
void QDX::ClassFactory<C>::addRecord(QDX::ClassRecord<C> *record)
{
	if (record == nullptr || m_records.contains(record)) {
		return;
	}
	m_records.insert(record);
	m_aliases[record->name()] = record;
	record->setFactory(this);
}

template <typename C>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::takeRecord(QDX::ClassRecord<C> *record)
{
	if (record == nullptr || m_records.contains(record) == false) {
		return record;
	}
	m_records.remove(record);
	foreach (const QString &key, m_aliases.keys(record)) {
		m_aliases.remove(key);
	}
	record->setFactory(nullptr);
	return record;
}

template <typename C>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::takeRecord(const QString &name)
{
	return this->takeRecord(this->record(name));
}

template<typename C>
void QDX::ClassFactory<C>::removeRecord(QDX::ClassRecord<C> *record)
{
	delete this->takeRecord(record);
}

template <typename C>
void QDX::ClassFactory<C>::removeRecord(const QString &name)
{
	delete this->takeRecord(name);
}

template<typename C>
bool QDX::ClassFactory<C>::contains(const QString &name) const
{
	return m_aliases.contains(name);
}

template<typename C>
bool QDX::ClassFactory<C>::contains(QDX::ClassRecord<C> *record) const
{
	return m_records.contains(record);
}

template <typename C>
C *QDX::ClassFactory<C>::instance(const QString &name) const
{
	QDX::ClassRecord<C> *record = this->record(name);
	if (record) {
		return record->instance();
	}
	return nullptr;
}

template <typename C>
void QDX::ClassFactory<C>::destroy(const QString &name, C *object) const
{
	QDX::ClassRecord<C> *record = this->record(name);
	if (record) {
		record->destroy(object);
	}
}

template<typename C>
void QDX::ClassFactory<C>::destroy(C *object) const
{
	QDX::ClassRecord<C> *record = this->findRecord(object);
	if (record) {
		record->destroy(object);
	}
}

template <typename C>
bool QDX::ClassFactory<C>::bind(const QString &alias, QDX::ClassRecord<C> *record)
{
	if (record == nullptr || record->factory() != this) {
		return false;
	}
	m_aliases[alias] = record;
	return true;
}

template <typename C>
bool QDX::ClassFactory<C>::bind(const QString &alias, const QString &name)
{
	return this->bind(alias, this->record(name));
}

template <typename C>
void QDX::ClassFactory<C>::unbind(const QString &name)
{
	m_aliases.remove(name);
}

template <typename C>
bool QDX::ClassFactory<C>::isRegistrationEnabled() const
{
	return m_registration_enabled;
}

template <typename C>
void QDX::ClassFactory<C>::setRegistration(bool registration)
{
	m_registration_enabled = registration;
}

template <typename C>
QDX::ClassRecord<C> *QDX::ClassFactory<C>::findRecord(const C *object) const
{
	for (const QDX::ClassRecord<C> *record : m_records) {
		if (record->instances().contains(object)) {
			return const_cast<QDX::ClassRecord<C> *>(record);
		}
	}
	return nullptr;
}

template <typename C>
QList<QDX::ClassRecord<C> *> QDX::ClassFactory<C>::records() const
{
	return m_records.toList();
}

#endif // QDX_METACLASSMANAGER_H