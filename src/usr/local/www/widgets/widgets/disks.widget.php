<?php
/*
 * disks.widget.php
 *
 * part of pfSense (https://www.pfsense.org)
 * Copyright (c) 2021-2023 Rubicon Communications, LLC (Netgate)
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Composer autoloader
require_once('vendor/autoload.php');

// pfSense includes
require_once('guiconfig.inc');

// Widget includes
require_once('/usr/local/www/widgets/include/disks.inc');

use pfSense\Services\Filesystem\{
	Filesystem,
	Filesystems,
	Provider\SystemProvider,
};


global $disks_widget_defaults;

$widgetkey = (isset($_POST['widgetkey'])) ? $_POST['widgetkey'] : $widgetkey;

// Now override any defaults with user settings
$widget_config = array_replace($disks_widget_defaults, (array) $user_settings['widgets'][$widgetkey]);

// Randomly invalidate the cache, 25% chance.
disks_cache_invalidate(false, 0.25);

// Are we handling an ajax refresh?
if (isset($_POST['ajax'])) {
	print(disks_compose_widget_table($widget_config));

	// We are done here...
	exit();
}

// Are we saving the configurable settings?
if (isset($_POST['save'])) {
	// Process settings post
	disks_do_widget_settings_post($_POST, $user_settings);

	// Redirect back to home...
	header('Location: /');

	// We are done here...
	exit();
}

$data = get_infomation_for_chart();
$data2 = get_infomation_for_chart3();
list($used, $size, $usedPercent, $used_kb, $size_kb) = json_decode($data, true);
list($path, $used2, $size2, $usedPercent2, $used_kb2, $size_kb2) = json_decode($data2, true);


?>


<script src="https://github.com/chartjs/Chart.js/releases/download/v2.9.3/Chart.min.js"></script>
<link rel="stylesheet" href="https://github.com/chartjs/Chart.js/releases/download/v2.9.3/Chart.min.css">

<!-- update ui -->
<div class="table-responsive" >

	<div class="row align-middle chart-container">
		<h5 class="chart-title">Mount: /</h5>
		<div class="col-sm-6">
			<div class="d-flex align-center">
				<div style="width: 100px; height: 100px;">
					<canvas class="chart" id="usersChart" width="1" height="1"> </canvas>
				</div>
				<div class="ml-3">
					<h5 style="margin-bottom: 2px; font-weight: 800"><?= $used ?></h5>
					<p>size <?= $size ?></p>
				</div>
			</div>
		</div>
		<div class="col-sm-6">
			<div class="d-flex align-center">
				<div style="width: 100px; height: 100px;">
					<canvas class="chart" id="usersChart2" width="1" height="1"></canvas>
				</div>
				<div class="ml-3">
					<h5 style="margin-bottom: 2px; font-weight: 800"><?= $usedPercent ?>%</h5>
					<p>size <?= $size ?></p>
				</div>
			</div>
		</div>
	</div>

	<div class="row align-middle chart-container">
		<h5 class="chart-title">Mount: <?= $path ?></h5>
		<div class="col-sm-6">
			<div class="d-flex align-center">
				<div style="width: 100px; height: 100px;">
					<canvas class="chart" id="usersChart3" width="1" height="1"> </canvas>
				</div>
				<div class="ml-3">
					<h5 style="margin-bottom: 2px; font-weight: 800"><?= $used2 ?></h5>
					<p>size <?= $size2 ?></p>
				</div>
			</div>
		</div>
		<div class="col-sm-6">
			<div class="d-flex align-center">
				<div style="width: 100px; height: 100px;">
					<canvas class="chart" id="usersChart4" width="1" height="1"></canvas>
				</div>
				<div class="ml-3">
					<h5 style="margin-bottom: 2px; font-weight: 800"><?= $usedPercent2 ?>%</h5>
					<p>size <?= $size2 ?></p>
				</div>
			</div>
		</div>
	</div>
</div>
</div>

<div id="widget-<?= htmlspecialchars($widgetkey) ?>_panel-footer" class="panel-footer collapse">

	<form action="/widgets/widgets/<?= $widgetconfig['basename'] ?>.widget.php" method="post" class="form-horizontal">
		<input type="hidden" name="widgetkey" value="<?= htmlspecialchars($widgetkey) ?>" />
		<input type="hidden" name="save" value="save" />
		<div class="panel panel-default col-sm-12">
			<div class="panel-body">
				<input type="hidden" name="widgetkey" value="<?= htmlspecialchars($widgetkey); ?>">
				<div class="table responsive">
					<table class="table table-striped table-hover table-condensed">
						<thead>
							<tr>
								<th>
									<?= htmlspecialchars(gettext("Mount")) ?>
								</th>
								<th>
									<span><i class="fa fa-thumb-tack" style="vertical-align:middle;"></i></span>
									<?= htmlspecialchars(gettext("Pin")) ?>
								</th>
							</tr>
						</thead>
						<tbody>
							<?php
							$disk_filter = explode(",", $user_settings['widgets'][$widgetkey]['disk_filter']);

							foreach (disks_get_nonroot_filesystems() as $fs) :
							?>
								<tr>
									<td>
										<?= htmlspecialchars($fs->getPath()) ?>
									</td>
									<td class="col-sm-2"><input id="<?= htmlspecialchars($widgetkey) ?>_disk_filter[]" name="<?= htmlspecialchars($widgetkey) ?>_disk_filter[]" value="<?= $fs->getPath() ?>" type="checkbox" <?= (in_array($fs->getPath(), $disk_filter) ? 'checked' : null) ?>></td>
								</tr>
							<?php
							endforeach;
							?>
						</tbody>
					</table>
				</div>
			</div>
		</div>

		<div class="form-group">
			<label for="<?= htmlspecialchars($widgetkey) ?>_autoshow_threshold" class="col-sm-4 control-label">
				<?= htmlspecialchars(gettext('Auto Show Threshold')) ?>
			</label>
			<div class="col-sm-8">
				<input type="number" id="<?= htmlspecialchars($widgetkey) ?>_autoshow_threshold" name="<?= htmlspecialchars($widgetkey) ?>_autoshow_threshold" value="<?= htmlspecialchars($widget_config['autoshow_threshold']) ?>" placeholder="<?= htmlspecialchars($disks_widget_defaults['autoshow_threshold']) ?>" min="0" max="100" class="form-control" />
				<span class="help-block">
					<?= htmlspecialchars(gettext('Automatically show mounts when utilization exceeds the specified threshold (%).')) ?>
					<br />
					<span class="text-danger">Note:</span>
					<?= sprintf(gettext('The default is %s%% (0%% to disable).'), htmlspecialchars($disks_widget_defaults['autoshow_threshold'])) ?>
				</span>
			</div>
		</div>

		<nav class="action-buttons">
			<button type="submit" class="btn btn-primary">
				<i class="fa fa-save icon-embed-btn"></i>
				<?= htmlspecialchars(gettext('Save')) ?>
			</button>
			<button id="<?= $widget_showallnone_id ?>" type="button" class="btn btn-info">
				<i class="fa fa-undo icon-embed-btn"></i>
				<?= htmlspecialchars(gettext('All')) ?>
			</button>
		</nav>
	</form>


	<script type="text/javascript">
		//<![CDATA[
		events.push(function() {
			let cookieName = <?= json_encode("treegrid-{$widgetkey}") ?>;

			// Callback function called by refresh system when data is retrieved
			function disks_callback(s) {
				var tree = $(<?= json_encode("#{$widgetkey}-table") ?>);

				tree.removeData();

				tree.html(s);

				initTreegrid(true);
			}

			// POST data to send via AJAX
			var postdata = {
				ajax: "ajax",
				widgetkey: <?= json_encode($widgetkey) ?>
			};

			// Create an object defining the widget refresh AJAX call
			var disksObject = new Object();
			disksObject.name = "disks";
			disksObject.url = "/widgets/widgets/disks.widget.php";
			disksObject.callback = disks_callback;
			disksObject.parms = postdata;
			disksObject.freq = 1;

			// Register the AJAX object
			register_ajax(disksObject);

			function initTreegrid(isAjax) {
				var tree = $(<?= json_encode("#{$widgetkey}-table") ?>);

				if (!isAjax) {
					$.removeCookie(cookieName);

					tree.removeData();
				}

				tree.treegrid({
					expanderExpandedClass: 'fa fa fa-chevron-down',
					expanderCollapsedClass: 'fa fa fa-chevron-right',
					initialState: 'collapsed',
					saveStateName: cookieName,
					saveState: true
				});
			}

			initTreegrid(false);

			set_widget_checkbox_events("#<?= $widget_panel_footer_id ?> [id^=<?= htmlspecialchars($widgetkey) ?>_disk_filter]", "<?= $widget_showallnone_id ?>");
		});
		//]]>
	</script>

	<script type="text/javascript">
		Chart.defaults.RoundedDoughnut = Chart.helpers.clone(Chart.defaults.doughnut);
		Chart.controllers.RoundedDoughnut = Chart.controllers.doughnut.extend({
			draw: function(ease) {
				var ctx = this.chart.ctx;
				var arcs = this.getMeta().data;
				var easingDecimal = ease || 1;
				Chart.helpers.each(arcs, function(arc, index) {
					arc.transition(easingDecimal).draw();
					var vm = arc._view;
					var radius = (vm.outerRadius + vm.innerRadius) / 2;
					var thickness = (vm.outerRadius - vm.innerRadius) / 2;
					var angle = Math.PI - vm.endAngle - Math.PI / 2;
					var startAngle = Math.PI - vm.startAngle - Math.PI / 2;
					var pArc = arcs[index === 0 ? arcs.length - 1 : index - 1];
					var pColor = pArc._view.backgroundColor;
					var text = arc._chart.options.centerText.text
					var fontColor = arc._chart.options.centerText.fontColor
					var fontSize = arc._chart.options.centerText.fontSize
					var font = arc._chart.options.centerText.font
					ctx.save();
					ctx.fillStyle = vm.backgroundColor;
					ctx.translate(vm.x, vm.y);
					ctx.beginPath();
					ctx.arc(radius * Math.sin(angle), radius * Math.cos(angle), thickness, 0, 2 * Math.PI);
					ctx.closePath();
					ctx.fill();
					ctx.fillStyle = index === 0 ? vm.backgroundColor : pColor;
					ctx.beginPath();
					ctx.arc(radius * Math.sin(startAngle), radius * Math.cos(startAngle), thickness, 0, 2 * Math.PI);
					ctx.arc(radius * Math.sin(Math.PI), radius * Math.cos(Math.PI), thickness, 0, 2 * Math.PI);
					ctx.fill();
					ctx.fillStyle = fontColor; // Đặt màu chữ
					ctx.font = fontSize + ' ' + font // Đặt kiểu chữ
					ctx.textBaseline = 'middle';
					ctx.textAlign = 'center';
					ctx.fillText(text, 0, 0);
					ctx.restore();
				});
			},
		});


		function createChart(canvas, progress, total, color, text, fontStyle, fontSize, fontColor) {
			chart = new Chart(document.getElementById(canvas), {
				type: 'RoundedDoughnut',
				data: {
					datasets: [{
						label: '123',
						data: [progress, total - progress],
						backgroundColor: [
							color,
							"rgba(84, 94, 107, 0.3)"
						],
						hoverBackgroundColor: [
							color,
							"rgba(84, 94, 107, 0.3)"
						],
						borderWidth: [
							0, 0
						]
					}]
				},

				options: {
					cutoutPercentage: 70,
					legend: {
						display: false
					},
					centerText: {
						display: true,
						text: text,
						fontSize: fontSize,
						fontColor: fontColor,
						font: fontStyle
					}
				}
			});
			return chart
		}
	</script>



	<script>
		data = <?= $data ?>;
		data2 = <?= $data2 ?>;
		console.log(data2)
		for (let i = 0; i < data.length; i++) {
			console.log(data[i])
			temp = data[i]
			used = temp['used']
			size = temp['size']
			usedPercent = temp['usedPercent']

			// if length of data > 1 uncomment this blow line and fix it
			// const chart1 = createChart(canvas = 'usersChart', progress = <?= $used ?>, total = <?= $size ?>, color = "#209AFF", text = "사용된", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");

		}
		// if length of data > 1 uncomment this blow line and fix it
		// for (let i = 0; i < data2.length; i++) {
		// 	console.log(data2[i])
		// 	temp = data2[i]
		// 	path = temp['path']
		// 	used2 = temp['used_kb']
		// 	size2 = temp['size_kb']
		// 	usedPercent2 = temp['usedPercent']
		// 	console.log(used2, size2)


		// 	const chart2 = createChart(canvas = 'usersChart2', progress = 17, total = 100, color = "#FB6159", text = "hello", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");

		// }
		const chart1 = createChart(canvas = 'usersChart', progress = <?= $used_kb ?>, total = <?= $size_kb ?>, color = "#209AFF", text = "사용된", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");
		const chart2 = createChart(canvas = 'usersChart2', progress = <?= $used_kb ?>, total = <?= $size_kb ?>, color = "#FB6159", text = "용법", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");
		const chart3 = createChart(canvas = 'usersChart3', progress = <?= $used_kb2 ?>, total = <?= $size_kb2 ?>, color = "#209AFF", text = "사용된", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");
		const chart4 = createChart(canvas = 'usersChart4', progress = <?= $used_kb2 ?>, total = <?= $size_kb2 ?>, color = "#FB6159", text = "용법", fillStyle = 'Fira Sans', fontSize = '14px', fontColor = "#ffff");
	</script>