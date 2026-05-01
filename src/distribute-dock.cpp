/* src/distribute-dock.cpp */
#include "distribute-dock.hpp"
#include "distribute-core.h"
#include "distribute-undo.h"
#include "plugin-support.h"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/bmem.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QVariant>
#include <QStyle>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QPalette>
#include <QApplication>
#include <QtSvg/QSvgRenderer>

namespace {

struct ButtonSpec {
	dist_action_t action;
	const char *labelKey;
	const char *tooltipKey;
	const char *iconFile;
};

const ButtonSpec kAlignButtons[] = {
	{DIST_ACTION_ALIGN_LEFT, "Align.Left", "Align.Left.Tip", "align-left.svg"},
	{DIST_ACTION_ALIGN_HCENTER, "Align.HCenter", "Align.HCenter.Tip", "align-hcenter.svg"},
	{DIST_ACTION_ALIGN_RIGHT, "Align.Right", "Align.Right.Tip", "align-right.svg"},
	{DIST_ACTION_ALIGN_TOP, "Align.Top", "Align.Top.Tip", "align-top.svg"},
	{DIST_ACTION_ALIGN_VCENTER, "Align.VCenter", "Align.VCenter.Tip", "align-vcenter.svg"},
	{DIST_ACTION_ALIGN_BOTTOM, "Align.Bottom", "Align.Bottom.Tip", "align-bottom.svg"},
};

const ButtonSpec kDistributeButtons[] = {
	{DIST_ACTION_DIST_H_LEFT_EDGES, "Dist.HLeft", "Dist.HLeft.Tip", "dist-h-left.svg"},
	{DIST_ACTION_DIST_H_CENTERS, "Dist.HCenter", "Dist.HCenter.Tip", "dist-h-center.svg"},
	{DIST_ACTION_DIST_H_RIGHT_EDGES, "Dist.HRight", "Dist.HRight.Tip", "dist-h-right.svg"},
	{DIST_ACTION_DIST_H_SPACING, "Dist.HSpacing", "Dist.HSpacing.Tip", "dist-h-spacing.svg"},
	{DIST_ACTION_DIST_V_TOP_EDGES, "Dist.VTop", "Dist.VTop.Tip", "dist-v-top.svg"},
	{DIST_ACTION_DIST_V_CENTERS, "Dist.VCenter", "Dist.VCenter.Tip", "dist-v-center.svg"},
	{DIST_ACTION_DIST_V_BOTTOM_EDGES, "Dist.VBottom", "Dist.VBottom.Tip", "dist-v-bottom.svg"},
	{DIST_ACTION_DIST_V_SPACING, "Dist.VSpacing", "Dist.VSpacing.Tip", "dist-v-spacing.svg"},
};

const ButtonSpec kSizeButtons[] = {
	{DIST_ACTION_SAME_WIDTH, "Size.SameWidth", "Size.SameWidth.Tip", "same-width.svg"},
	{DIST_ACTION_SAME_HEIGHT, "Size.SameHeight", "Size.SameHeight.Tip", "same-height.svg"},
	{DIST_ACTION_SAME_SIZE, "Size.SameSize", "Size.SameSize.Tip", "same-size.svg"},
};

const ButtonSpec kRandomButtons[] = {
	{DIST_ACTION_RANDOM_SCATTER, "Random.Scatter", "Random.Scatter.Tip", "random-scatter.svg"},
};

QIcon loadThemedSvgIcon(const char *filename)
{
	QByteArray relPath = QByteArray("icons/") + filename;
	char *fullPath = obs_module_file(relPath.constData());
	if (!fullPath)
		return QIcon();

	QFile file(QString::fromUtf8(fullPath));
	bfree(fullPath);
	if (!file.open(QIODevice::ReadOnly))
		return QIcon();

	QByteArray svg = file.readAll();
	file.close();

	QColor color = QApplication::palette().color(QPalette::ButtonText);
	svg.replace("currentColor", color.name().toUtf8());

	QSvgRenderer renderer(svg);
	if (!renderer.isValid())
		return QIcon();

	QPixmap pm(64, 64);
	pm.fill(Qt::transparent);
	QPainter painter(&pm);
	painter.setRenderHint(QPainter::Antialiasing, true);
	renderer.render(&painter);
	painter.end();
	return QIcon(pm);
}

bool is_align(dist_action_t a)
{
	return a >= DIST_ACTION_ALIGN_LEFT && a <= DIST_ACTION_ALIGN_BOTTOM;
}

bool is_distribute(dist_action_t a)
{
	return a >= DIST_ACTION_DIST_H_LEFT_EDGES &&
	       a <= DIST_ACTION_DIST_V_SPACING;
}

} // namespace

DistributeDock::DistributeDock(QWidget *parent) : QWidget(parent)
{
	setObjectName("DistributeDock");

	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(8, 8, 8, 8);
	root->setSpacing(14);

	auto makeRow = [this](const auto &specs, QList<QToolButton *> &out) {
		auto *row = new QHBoxLayout();
		row->setContentsMargins(0, 0, 0, 0);
		row->setSpacing(6);
		for (const auto &spec : specs) {
			QToolButton *btn = makeActionButton(
				spec.action, spec.labelKey, spec.tooltipKey);
			row->addWidget(btn);
			out.append(btn);
		}
		row->addStretch(1);
		return row;
	};

	root->addLayout(makeRow(kAlignButtons, m_alignButtons));
	root->addLayout(makeRow(kDistributeButtons, m_distributeButtons));
	root->addLayout(makeRow(kSizeButtons, m_sizeButtons));
	root->addLayout(makeRow(kRandomButtons, m_randomButtons));
	root->addSpacing(4);

	m_statusLabel = new QLabel(this);
	m_statusLabel->setAlignment(Qt::AlignCenter);
	root->addWidget(m_statusLabel);

	auto *timer = new QTimer(this);
	timer->setInterval(250);
	connect(timer, &QTimer::timeout, this,
		&DistributeDock::refreshSelectionCount);
	timer->start();

	refreshSelectionCount();
}

DistributeDock::~DistributeDock() = default;

QToolButton *DistributeDock::makeActionButton(dist_action_t action,
					      const char *labelKey,
					      const char *tooltipKey)
{
	const ButtonSpec *spec = nullptr;
	for (const auto &s : kAlignButtons)
		if (s.action == action) spec = &s;
	for (const auto &s : kDistributeButtons)
		if (s.action == action) spec = &s;
	for (const auto &s : kSizeButtons)
		if (s.action == action) spec = &s;
	for (const auto &s : kRandomButtons)
		if (s.action == action) spec = &s;

	auto *btn = new QToolButton(this);
	btn->setToolTip(obs_module_text(tooltipKey));
	btn->setAutoRaise(true);
	btn->setProperty("dist_action", static_cast<int>(action));
	btn->setFixedSize(44, 44);
	btn->setIconSize(QSize(28, 28));
	btn->setFocusPolicy(Qt::NoFocus);

	QIcon icon = spec ? loadThemedSvgIcon(spec->iconFile) : QIcon();
	if (!icon.isNull()) {
		btn->setIcon(icon);
		btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	} else {
		btn->setText(obs_module_text(labelKey));
		btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
	}

	connect(btn, &QToolButton::clicked, this,
		&DistributeDock::onActionClicked);
	return btn;
}

void DistributeDock::updateEnabledState(int selected)
{
	const bool alignOk = selected >= 2;
	const bool distOk = selected >= 3;
	const bool sizeOk = selected >= 2;
	const bool randomOk = selected >= 1;

	for (auto *b : m_alignButtons)
		b->setEnabled(alignOk);
	for (auto *b : m_distributeButtons)
		b->setEnabled(distOk);
	for (auto *b : m_sizeButtons)
		b->setEnabled(sizeOk);
	for (auto *b : m_randomButtons)
		b->setEnabled(randomOk);
}

void DistributeDock::refreshSelectionCount()
{
	obs_source_t *scene_src = obs_frontend_get_current_scene();
	int n = 0;
	if (scene_src) {
		obs_scene_t *scene = obs_scene_from_source(scene_src);
		if (scene)
			n = dist_count_selected(scene);
		obs_source_release(scene_src);
	}
	m_statusLabel->setText(
		QString(obs_module_text("Status.ItemsSelected")).arg(n));
	updateEnabledState(n);
}

void DistributeDock::onActionClicked()
{
	auto *btn = qobject_cast<QToolButton *>(sender());
	if (!btn)
		return;

	dist_action_t action = static_cast<dist_action_t>(
		btn->property("dist_action").toInt());

	obs_source_t *scene_src = obs_frontend_get_current_scene();
	if (!scene_src)
		return;
	obs_scene_t *scene = obs_scene_from_source(scene_src);
	if (!scene) {
		obs_source_release(scene_src);
		return;
	}

	int selected = dist_count_selected(scene);
	int min_needed;
	if (action == DIST_ACTION_RANDOM_SCATTER)
		min_needed = 1;
	else if (is_distribute(action))
		min_needed = 3;
	else
		min_needed = 2;
	if (selected < min_needed) {
		obs_source_release(scene_src);
		return;
	}

	char *undo_json = dist_snapshot_selected(scene);
	bool ok = dist_apply(scene, action);

	if (ok) {
		char *redo_json = dist_snapshot_selected(scene);
		const char *undo_label_key;
		if (action == DIST_ACTION_RANDOM_SCATTER)
			undo_label_key = "Undo.Random";
		else if (is_align(action))
			undo_label_key = "Undo.Align";
		else if (is_distribute(action))
			undo_label_key = "Undo.Distribute";
		else
			undo_label_key = "Undo.SameSize";
		obs_frontend_add_undo_redo_action(
			obs_module_text(undo_label_key), dist_undo_apply,
			dist_undo_apply, undo_json, redo_json, false);
		bfree(redo_json);
	} else {
		obs_log(LOG_WARNING, "action %d failed", (int)action);
	}

	bfree(undo_json);
	obs_source_release(scene_src);
}

extern "C" void obs_distribute_register_dock(void)
{
	auto *dock = new DistributeDock();
	obs_frontend_add_dock_by_id("obs-distribute-dock",
				    obs_module_text("Dock.Title"), dock);
}
