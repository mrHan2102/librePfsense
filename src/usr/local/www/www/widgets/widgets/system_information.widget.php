<?php
/*
 * system_information.widget.php
 *
 * part of pfSense (https://www.pfsense.org)
 * Copyright (c) 2004-2013 BSD Perimeter
 * Copyright (c) 2013-2016 Electric Sheep Fencing
 * Copyright (c) 2014-2022 Rubicon Communications, LLC (Netgate)
 * Copyright (c) 2007 Scott Dale
 * All rights reserved.
 *
 * originally part of m0n0wall (http://m0n0.ch/wall)
 * Copyright (c) 2003-2004 Manuel Kasper <mk@neon1.net>.
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

require_once("functions.inc");
require_once("guiconfig.inc");
require_once('notices.inc');
require_once('system.inc');
include_once("includes/functions.inc.php");

$sysinfo_items = array(
	'name' => gettext('Name'),
	'user' => gettext('User'),
	'system' => gettext('System'),
	'bios' => gettext('BIOS'),
	'version' => gettext('Version'),
	'cpu_type' => gettext('CPU Type'),
	'hwcrypto' => gettext('Hardware Crypto'),
	'pti' => gettext('Kernel PTI'),
	'mds' => gettext('MDS Mitigation'),
	'uptime' => gettext('Uptime'),
	'current_datetime' => gettext('Current Date/Time'),
	'dns_servers' => gettext('DNS Server(s)'),
	'last_config_change' => gettext('Last Config Change'),
	'state_table_size' => gettext('State Table Size'),
	'mbuf_usage' => gettext('MBUF Usage'),
	'temperature' => gettext('Temperature'),
	'load_average' => gettext('Load Average'),
	'cpu_usage' => gettext('CPU Usage'),
	'memory_usage' => gettext('Memory Usage'),
	'swap_usage' => gettext('Swap Usage')
	);

// Declared here so that JavaScript can access it
$updtext = sprintf(gettext("Obtaining update status %s"), "<i class='fa fa-cog fa-spin'></i>");
$state_tt = gettext("Adaptive state handling is enabled, state timeouts are reduced by ");

if ($_REQUEST['getupdatestatus']) {
	require_once("pkg-utils.inc");

	$cache_file = $g['version_cache_file'];

	if (isset($config['system']['firmware']['disablecheck'])) {
		exit;
	}

	/* If $_REQUEST['getupdatestatus'] == 2, force update */
	$system_version = get_system_pkg_version(false,
	    ($_REQUEST['getupdatestatus'] == 1));

	if ($system_version === false) {
		print(gettext("<i>Unable to check for updates</i>"));
		exit;
	}

	if (!is_array($system_version) ||
	    !isset($system_version['version']) ||
	    !isset($system_version['installed_version'])) {
		print(gettext("<i>Error in version information</i>"));
		exit;
	}

	switch ($system_version['pkg_version_compare']) {
	case '<':
?>
		<div>
			<?=gettext("Version ")?>
			<span class="text-success"><?=$system_version['version']?></span> <?=gettext("is available.")?>
			<a class="fa fa-cloud-download fa-lg" href="/pkg_mgr_install.php?id=firmware"></a>
		</div>
<?php
		break;
	case '=':
		printf('<span class="text-success">%s</span>' . "\n",
		    gettext("The system is on the latest version."));
		break;
	case '>':
		printf("%s\n", gettext(
		    "The system is on a later version than official release."));
		break;
	default:
		printf("<i>%s</i>\n", gettext(
		    "Error comparing installed with latest version available"));
		break;
	}

	if (file_exists($cache_file)):
?>
	<div>
		<?printf("%s %s", gettext("Version information updated at"),
		    date("D M j G:i:s T Y", filemtime($cache_file)));?>
		    &nbsp;
		    <a id="updver" href="#" class="fa fa-refresh"></a>
	</div>
<?php
	endif;

	exit;
} elseif ($_POST['widgetkey']) {
	set_customwidgettitle($user_settings);

	$validNames = array();

	foreach ($sysinfo_items as $sysinfo_item_key => $sysinfo_item_name) {
		array_push($validNames, $sysinfo_item_key);
	}

	if (is_array($_POST['show'])) {
		$user_settings['widgets'][$_POST['widgetkey']]['filter'] = implode(',', array_diff($validNames, $_POST['show']));
	} else {
		$user_settings['widgets'][$_POST['widgetkey']]['filter'] = implode(',', $validNames);
	}

	save_widget_settings($_SESSION['Username'], $user_settings["widgets"], gettext("Saved System Information Widget Filter via Dashboard."));
	header("Location: /index.php");
}

$hwcrypto = get_cpu_crypto_support();

$skipsysinfoitems = explode(",", $user_settings['widgets'][$widgetkey]['filter']);

$rows_displayed = false;
// use the preference of the first thermal sensor widget, if it's available (false == empty)
$temp_use_f = (isset($user_settings['widgets']['thermal_sensors-0']) && !empty($user_settings['widgets']['thermal_sensors-0']['thermal_sensors_widget_show_fahrenheit']));
?>
 <!-- update ui system information -->

<div class = "box-system-information">
	<!-- <div class = "title">
		<h2>시스템 정보</h2>
	</div> -->
	<div class = "container-system-information">
		<div class = "system-charts">
			<div class="chart">
				<canvas id="memory" class="chart-info"></canvas>
				<?php $memUsage = mem_usage(); ?>
				<div class="chart-content"><span class="storage-using"><?=$memUsage?>% </span><br> of <?= sprintf("%.0f", get_single_sysctl('hw.physmem') / (1024*1024)) ?> MiB</div>
			</div>
			<div class="chart">
				<canvas id="cpu" class="chart-info"></canvas>
				<?php $cpuUsage = get_cpu_ticks(); ?>
				<?php $cpuTotal = get_cpu_ticks_total(); ?>
				<div class="chart-content"><span class="storage-using"> <?=$cpuUsage?> % </span><br>of <?=$cpuTotal?> <br> CPU Ticks</div>
			</div>
			<div class="chart">
				<canvas id="swap" class="chart-info"></canvas>
				<?php $swapUsage = swap_usage(); ?>
				<div class="chart-content"><span class="storage-using"><?=$swapUsage?>% </span><br> of <?= sprintf("%.0f", `/usr/sbin/swapinfo -m | /usr/bin/tail -1 | /usr/bin/awk '{ print $2;}'`) ?> MiB</div>
			</div>
		</div>
		<div class="system-items-1">
			<div class="item-1">
				<?php if (!in_array('uptime', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="title-uptime"><?=gettext("Uptime: ");?> &nbsp; </div>
					<div class="content-uptime"><?= htmlspecialchars(get_uptime()); ?></div>
				<?php endif; ?>
			</div>
			<div class="item-1">
				<?php if (!in_array('load_average', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="title-avg"><?=gettext("Load average: ");?> &nbsp; </div>
					<div class="content-avg"><?= get_load_average(); ?></div>
				<?php endif; ?>
			</div>
		</div>
		<div class = "system-charts">
			<div class = "chart">
				<canvas id="state-table" class = "chart-info-2"></canvas>
				<?php $pfstatetext = get_pfstate();
				$pfstateusage = get_pfstate(true); ?>
				<div class = "chart-content-2">
					<span class="storage-using"><?=$pfstateusage?>% </span><br> 
					<span class="estimate" id="pfstate"><?= htmlspecialchars($pfstatetext)?></span><br>
					<button class="btn-state-table"><a href="diag_dump_states.php"><?=gettext("View Detail");?></a></button>
				</div>
			</div>
			<div class="chart">
				<canvas id="mbuf" class="chart-info-2"></canvas>
				<?php get_mbuf($mbufstext, $mbufusage); ?>
				<div class="chart-content-2"><span class="storage-using"><?=$mbufusage?>% </span><br> <span class="estimate" id="mbuf"><?= htmlspecialchars($mbufstext)?></div>
			</div>
		</div>
		<div class = "check-status">
			<div class="status">
				<?php $pti = get_single_sysctl('vm.pmap.pti');
					if ((strlen($pti) > 0) && !in_array('pti', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<?php if ($pti == 0): ?>
						<img class="icon-status" src="./active.svg" alt="">
					<?php else: ?>
						<img class="icon-status"src="./inactive.svg" alt="">
					<?php endif; ?>
					<div class = "status-name"><?=gettext("Kernel PTI");?></div>
				<?php endif; ?>
			</div>
			<div class = "status">
				<?php $mds = get_single_sysctl('hw.mds_disable_state');
					if ((strlen($mds) > 0) && !in_array('mds', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<?php if (ucwords(htmlspecialchars($mds)) == 'Inactive'): ?>
						<img class="icon-status" src="./inactive.svg" alt="">
					<?php else: ?>
						<img class="icon-status"src="./active.svg" alt="">
					<?php endif; ?>
					<div class = "status-name"><?=gettext("MDS Mitigation");?></div>
				<?php endif; ?>
			</div>
			<div class = "status">
				<?php if (!in_array('hwcrypto', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<?php if ($hwcrypto): ?>
					<?php if (htmlspecialchars(crypto_accel_get_algs($hwcrypto)) == ''): ?>
						<img class="icon-status" src="./inactive.svg" alt="">
					<?php else: ?>
						<img class="icon-status"src="./active.svg" alt="">
					<?php endif; ?>
					<div class = "status-name"><?=gettext("Hardware crypto");?></div>
					<?php endif; ?>
				<?php endif; ?>
			</div>
		</div>
		<div class="system-items-2">
			<div class="item-2">
				<?php if (!in_array('name', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="item-title"><?=gettext("Name:");?></div>
					<div class="item-content"><?php echo htmlspecialchars($config['system']['hostname'] . "." . $config['system']['domain']); ?></div>
				<?php endif; ?>
			</div>
			<div class="item-2">
				<?php if (!in_array('user', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="item-title"><?=gettext("User:");?></div>
					<div class="item-content"><?php echo htmlspecialchars(get_config_user()); ?></div>
				<?php endif; ?>
			</div>
			<div class="item-2">
				<?php if (!in_array('system', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="item-title"><?=gettext("System:");?></div>
					<div class="item-content">
						<?php 
							$platform = system_identify_specific_platform();
							if (isset($platform['descr'])) {
								echo $platform['descr'];
							} else {
								echo gettext('Unknown system');
							}
			
							$serial = system_get_serial();
							if (!empty($serial)) {
								print("<br />" . gettext("Serial:") .
									" <strong>{$serial}</strong>\n");
							}
			
							// If the uniqueID is available, display it here
							$uniqueid = system_get_uniqueid();
							if (!empty($uniqueid)) {
								print("<br />" .
									gettext("Netgate Device ID:") .
									" <strong>{$uniqueid}</strong>");
							} 
						?>
					</div>
				<?php endif; ?>
			</div>
			<div class="item-2">
				<?php if (!in_array('bios', $skipsysinfoitems)):
					$rows_displayed = true;
					unset($biosvendor);
					unset($biosversion);
					unset($biosdate);
					$_gb = exec('/bin/kenv -q smbios.bios.vendor 2>/dev/null', $biosvendor);
					$_gb = exec('/bin/kenv -q smbios.bios.version 2>/dev/null', $biosversion);
					$_gb = exec('/bin/kenv -q smbios.bios.reldate 2>/dev/null', $biosdate);
					/* Only display BIOS information if there is any to show. */
					if (!empty($biosvendor[0]) || !empty($biosversion[0]) || !empty($biosdate[0])): ?>
					<div class="item-title"><?=gettext("BIOS:");?></div>
					<div class="item-content">
						<?php if (!empty($biosvendor[0])): ?>
						<?=gettext("Vendor: ");?><strong><?=$biosvendor[0];?></strong><br/>
						<?php endif; ?>
						<?php if (!empty($biosversion[0])): ?>
							<?=gettext("Version: ");?><strong><?=$biosversion[0];?></strong><br/>
						<?php endif; ?>
						<?php if (!empty($biosdate[0])): ?>
							<?=gettext("Release Date: ");?><strong><?= date("D M j Y ",strtotime($biosdate[0]));?></strong><br/>
						<?php endif; ?>
					</div>
				<?php endif; endif; ?>
			</div>
			<div class="item-2">
				<?php if (!in_array('version', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="item-title"><?=gettext("Version:");?></div>
					<div class="item-content">
						<strong><?=$g['product_version_string']?></strong>
						(<?php echo php_uname("m"); ?>)
						<br />
						<?=gettext('built on')?> <?php readfile("/etc/version.buildtime"); ?>
						<?php if (!$g['hideuname']): ?>
							<br />
							<span title="<?php echo php_uname("a"); ?>"><?php echo php_uname("s") . " " . php_uname("r"); ?></span>
						<?php endif; ?>
						<?php if (!isset($config['system']['firmware']['disablecheck'])): ?>
							<br />
							<div id='updatestatus'><?=$updtext?></div>
						<?php endif; ?>
					</div>
				<?php endif; ?>
			</div>
			<div class="item-2">
				<?php if (!in_array('cpu_type', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<div class="item-title"><?=gettext("CPU Type:");?></div>
					<div class="item-content">
						<?=htmlspecialchars(get_single_sysctl("hw.model"))?>
						<div id="cpufreq"><?= get_cpufreq(); ?></div>
						<?php
							$cpucount = get_cpu_count();
							if ($cpucount > 1): ?>
								<div id="cpucount">
									<?= htmlspecialchars($cpucount) ?> <?=gettext('CPUs')?>: <?= htmlspecialchars(get_cpu_count(true)); ?>
								</div>
						<?php endif; ?>
							<div id="cpucrypto">
								<?= get_cpu_crypto_string($hwcrypto); ?>
							</div>
					</div>
				<?php endif; ?>
			</div>
		</div>
		<div class = "last-config">
			<div class="item-1">
				<?php if (!in_array('last_config_change', $skipsysinfoitems)):
					$rows_displayed = true; ?>
					<?php if ($config['revision']): ?>
					<div class="title-uptime"><?=gettext("Last config change: ");?> &nbsp; </div>
					<div class="content-uptime"><?= htmlspecialchars(date("D M j G:i:s T Y", intval($config['revision']['time'])));?></div>
					<?php endif; ?>
				<?php endif; ?>
			</div>
		</div>
	</div>

</div>
<!-- close the body we're wrapped in and add a configuration-panel -->
</div><div id="<?=$widget_panel_footer_id?>" class="panel-footer collapse">

<form action="/widgets/widgets/system_information.widget.php" method="post" class="form-horizontal">
    <div class="panel panel-default col-sm-10">
		<div class="panel-body">
			<input type="hidden" name="widgetkey" value="<?=htmlspecialchars($widgetkey); ?>">
			<div class="table responsive">
				<table class="table table-striped table-hover table-condensed">
					<thead>
						<tr>
							<th><?=gettext("Item")?></th>
							<th><?=gettext("Show")?></th>
						</tr>
					</thead>
					<tbody>
<?php
				foreach ($sysinfo_items as $sysinfo_item_key => $sysinfo_item_name):
?>
						<tr>
							<td><?=gettext($sysinfo_item_name)?></td>
							<td class="col-sm-2"><input id="show[]" name ="show[]" value="<?=$sysinfo_item_key?>" type="checkbox" <?=(!in_array($sysinfo_item_key, $skipsysinfoitems) ? 'checked':'')?>></td>
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
		<div class="col-sm-offset-3 col-sm-6">
			<button type="submit" class="btn btn-primary"><i class="fa fa-save icon-embed-btn"></i><?=gettext('Save')?></button>
			<button id="<?=$widget_showallnone_id?>" type="button" class="btn btn-info"><i class="fa fa-undo icon-embed-btn"></i><?=gettext('All')?></button>
		</div>
	</div>
</form>
<script src="https://github.com/chartjs/Chart.js/releases/download/v2.9.3/Chart.min.js"></script>
<script>

</script>
<script>
//<![CDATA[
using_cpu =0;
console.log("djs", using_cpu);
<?php if ($widget_first_instance): ?>

var lastTotal = 0;
var lastUsed = 0;


// Collect some PHP values required by the states calculation
<?php if (!in_array('state_table_size', $skipsysinfoitems)): ?>
var adaptiveend = <?=$adaptiveend?>;
var adaptivestart = <?=$adaptivestart?>;
var maxstates = <?=$maxstates?>;
var state_tt = "<?=$state_tt?>";
<?php else: ?>
var adaptiveend = 0;
var adaptivestart = 0;
var maxstates = 0;
var state_tt = "";
<?php endif; ?>

function setProgress(barName, percent) {
	$('[id="' + barName + '"]').css('width', percent + '%').attr('aria-valuenow', percent);
}

function stats(x) {
	var values = x.split("|");
	if ($.each(values,function(key,value) {
		if (value == 'undefined' || value == null)
			return true;
		else
			return false;
	}))

	if (lastTotal === 0) {
		lastTotal = values[0];
		lastUsed = values[1];
	} else {
		updateCPU(values[0], values[1]);
	}

	updateUptime(values[3]);
	updateDateTime(values[6]);
	updateMemory(values[2]);
	updateState(values[4]);
	updateTemp(values[5]);
	updateCpuFreq(values[7]);
	updateLoadAverage(values[8]);
	updateMbuf(values[9]);
	updateMbufMeter(values[10]);
	updateStateMeter(values[11]);
}

function updateMemory(x) {
	if ($('#memusagemeter')) {
		$('[id="memusagemeter"]').html(x);
	}
	if ($('#memUsagePB')) {
		setProgress('memUsagePB', parseInt(x));
	}
}

function updateMbuf(x) {
	if ($('#mbuf')) {
		$('[id="mbuf"]').html('(' + x + ')');
	}
}

function updateMbufMeter(x) {
	if ($('#mbufusagemeter')) {
		$('[id="mbufusagemeter"]').html(x + '%');
	}
	if ($('#mbufPB')) {
		setProgress('mbufPB', parseInt(x));
	}
}

function updateCPU(total, used) {
	if ((lastTotal <= total) && (lastUsed <= used)) { // Just in case it wraps
		// Calculate the total ticks and the used ticks since the last time it was checked
		var d_total = total - lastTotal;
		var d_used = used - lastUsed;

		// Convert to percent
		var x = Math.floor(((d_total - d_used)/d_total) * 100);
		// using_cpu = x;
		// console.log("djs", using_cpu);
		if ($('#cpumeter')) {
			$('[id="cpumeter"]').html(x + '%');
		}

		if ($('#cpuPB')) {
			setProgress('cpuPB', parseInt(x));
		}

		/* Load CPU Graph widget if enabled */
		if (widgetActive('cpu_graphs')) {
			GraphValue(graph[0], x);
		}
	}

	// Update the saved "last" values
	lastTotal = total;
	lastUsed = used;
}

function updateTemp(x) {
	$("#tempmeter").html(function() {
		return this.dataset.units === "F" ? parseInt(x * 1.8 + 32, 10) : x;
	});
	setProgress('tempPB', parseInt(x));
}

function updateDateTime(x) {
	if ($('#datetime')) {
		$('[id="datetime"]').html(x);
	}
}

function updateUptime(x) {
	if ($('#uptime')) {
		$('[id="uptime"]').html(x);
	}
}

function updateState(x) {
	if ($('#pfstate')) {
		$('[id="pfstate"]').html('(' + x + ')');

		// get numeric part of string before hte '/'
		x = x.split('/')[0]

		if (x > adaptivestart) {
			var scalingfactor = Math.round((adaptiveend - x) / (adaptiveend - adaptivestart) * 100);
			var disphtml = 	'<br /><a href="#" data-toggle="tooltip" title="" data-placement="right" data-original-title="' +
				state_tt +  scalingfactor + '%">' +
				'Scaling ' + scalingfactor + '%</a>';

			// Only update the display if the tooltip is not visible. Otherwise the tip will go away
			if ($('.tooltip').length == 0) {
				$('#scaledstates').html(disphtml);
			}

			// Renable the tooltip
			$(function () {
				$('[data-toggle="tooltip"]').tooltip()
			})

			$('#statePB').addClass('progress-bar-warning');
		} else {
			$('#scaledstates').html('');
			$('#statePB').removeClass('progress-bar-warning');
		}
	}
}

function updateStateMeter(x) {
	if ($('#pfstateusagemeter')) {
		$('[id="pfstateusagemeter"]').html(x + '%');
	}
	if ($('#statePB')) {
		setProgress('statePB', parseInt(x));
	}
}

function updateCpuFreq(x) {
	if ($('#cpufreq')) {
		$('[id="cpufreq"]').html(x);
	}
}

function updateLoadAverage(x) {
	if ($('#load_average')) {
		$('[id="load_average"]').html(x);
	}
}

function widgetActive(x) {
	var widget = $('#' + x + '-container');
	if ((widget != null) && (widget.css('display') != null) && (widget.css('display') != "none")) {
		return true;
	} else {
		return false;
	}
}


<?php endif; // $widget_first_instance ?>


events.push(function() {

	$('#scaledstates').html('');

	// Enable tooltips
	$(function () {
		$('[data-toggle="tooltip"]').tooltip()
	})

	// --------------------- Centralized widget refresh system ------------------------------

	// Callback function called by refresh system when data is retrieved
	function meters_callback(s) {
		stats(s);
	}

	// POST data to send via AJAX
	var postdata = {
		ajax: "ajax",
		skipitems: <?=json_encode($skipsysinfoitems)?>
	 };

	// Create an object defining the widget refresh AJAX call
	var metersObject = new Object();
	metersObject.name = "Meters";
	metersObject.url = "/getstats.php";
	metersObject.callback = meters_callback;
	metersObject.parms = postdata;
	metersObject.freq = 1;

	// Register the AJAX object
	register_ajax(metersObject);

<?php if (!isset($config['system']['firmware']['disablecheck'])): ?>

	// Callback function called by refresh system when data is retrieved
	function version_callback(s) {
		$('[id^=widget-system_information] #updatestatus').html(s);

		// The click handler has to be attached after the div is updated
		$('#updver').click(function() {
			updver_ajax();
		});
	}

	// POST data to send via AJAX
	var postdata = {
		ajax: "ajax",
		getupdatestatus: "1",
		skipitems: <?=json_encode($skipsysinfoitems)?>
	 };

	// Create an object defining the widget refresh AJAX call
	var versionObject = new Object();
	versionObject.name = "Version";
	versionObject.url = "/widgets/widgets/system_information.widget.php";
	versionObject.callback = version_callback;
	versionObject.parms = postdata;
	versionObject.freq = 100;

	//Register the AJAX object
	register_ajax(versionObject);
<?php endif; ?>
	// ---------------------------------------------------------------------------------------------------

	set_widget_checkbox_events("#<?=$widget_panel_footer_id?> [id^=show]", "<?=$widget_showallnone_id?>");

	// AJAX function to update the version display with non-cached data
	function updver_ajax() {

		// Display the "updating" message
		$('[id^=widget-system_information] #updatestatus').html("<?=$updtext?>"); // <?=$updtext?>");

		$.ajax({
			type: 'POST',
			url: "/widgets/widgets/system_information.widget.php",
			dataType: 'html',
			data: {
				ajax: "ajax",
				getupdatestatus: "2",
				skipitems: <?=json_encode($skipsysinfoitems)?>
			},

			success: function(data){
				// Display the returned data
				$('[id^=widget-system_information] #updatestatus').html(data);

				// Re-attach the click handler (The binding was lost when the <div> content was replaced)
				$('#updver').click(function() {
					updver_ajax();
				});
			},

			error: function(e){
			}
		});
	}
});

//]]>
</script>
<script>
// create round cornner
Chart.defaults.RoundedDoughnut = Chart.helpers.clone(Chart.defaults.doughnut);
	Chart.controllers.RoundedDoughnut = Chart.controllers.doughnut.extend({
		draw: function (ease) {
			var ctx = this.chart.ctx;
			var arcs = this.getMeta().data;
			var easingDecimal = ease || 1;
			Chart.helpers.each(arcs, function (arc, index) {
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
				ctx.font = fontSize + ' '+font; // Đặt kiểu chữ
				ctx.textBaseline = 'middle';
				ctx.textAlign = 'center';
				ctx.fillText(text, 0, 0);
				ctx.restore();

			});
		},

	});

	// draw chart
	function drawChart(id, text, using, free, fontSize, color){
		chart = new Chart(document.getElementById(id), {
			type: 'RoundedDoughnut',
			data: {
				datasets: [
					{
						label: 'Value',
						data: [using, free],
						backgroundColor: [
							color,
							'#545E6B'
						],
						
						borderWidth: 0
					}]
			},
			options: {
				cutoutPercentage: 65,
				legend: {
					display: false
				},
				centerText: {
					display: true,
					text: text,
					fontSize: fontSize,
					fontColor: 'white',
					font: 'sans-serif',
					lineHeight: 1.5
				}
			}
		});
		console.log(text);
		return chart
	}
<? $mem_usage = mem_usage(); ?>
data111= <?=$mem_usage?>;
console.log("dsasj", data111);
<?php $swap_usage = swap_usage(); ?>
<?php $pf_state_text = get_pfstate(); $pf_state_usage = get_pfstate(true); ?>
<?php get_mbuf($mbuf_stext, $mbuf_usage); ?>
<?php $cpu_unit = get_cpu_ticks(); ?>
<?php $cpu_total = get_cpu_ticks_total(); ?>
	var chartMemory = drawChart(id = 'memory', text = 'Memory', using = <?=$mem_usage?>, free = 100 - <?=$mem_usage?>, fontSize = '15px', color = '#34D399')
	var chartCPU = drawChart(id = 'cpu', text = 'CPU', using = <?=$cpu_unit?>, free = <?=$cpu_total?> - <?=$cpu_unit?>, fontSize = '15px', color = '#209AFF')
	var chartSWAP = drawChart(id = 'swap', text = 'SWAP', using = <?=$swap_usage?>, free = 100 - <?=$swap_usage?>, fontSize = '15px', color = '#FB6159' )
	var chartStateTable = drawChart(id = 'state-table', text = 'State\ntable', using = <?=$pf_state_usage?>, free = 100 - <?=$pf_state_usage?>, fontSize = '12px', color = '#209AFF')
	var charMBUF = drawChart(id = 'mbuf', text = 'MBUF usage', using = <?=$mbuf_usage?>, free = 100 - <?=$mbuf_usage?>, fontSize = '12px', color = '#209AFF')

// finish draw chart
</script>
