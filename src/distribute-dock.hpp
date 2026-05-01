/* src/distribute-dock.hpp */
#pragma once

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QToolButton>

#include "distribute-core.h"

class DistributeDock : public QWidget {
	Q_OBJECT

public:
	explicit DistributeDock(QWidget *parent = nullptr);
	~DistributeDock() override;

private slots:
	void onActionClicked();
	void refreshSelectionCount();

private:
	QToolButton *makeActionButton(dist_action_t action, const char *labelKey,
				      const char *tooltipKey);
	void updateEnabledState(int selected);

	QLabel *m_statusLabel = nullptr;
	QList<QToolButton *> m_alignButtons;
	QList<QToolButton *> m_distributeButtons;
	QList<QToolButton *> m_sizeButtons;
	QList<QToolButton *> m_randomButtons;
};
