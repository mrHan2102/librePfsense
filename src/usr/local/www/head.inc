<?php
/*
 * head.inc
 *
 * part of pfSense (https://www.pfsense.org)
 * Copyright (c) 2004-2013 BSD Perimeter
 * Copyright (c) 2013-2016 Electric Sheep Fencing
 * Copyright (c) 2014-2022 Rubicon Communications, LLC (Netgate)
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

require_once("globals.inc");
require_once("functions.inc");
require_once("shortcuts.inc");
require_once("service-utils.inc");
require_once('notices.inc');

header('Content-Type: text/html; charset=utf-8');

$pagetitle = gentitle($pgtitle);
$system_url = 'PSS';

if ($user_settings['webgui']['pagenamefirst']) {
	$tabtitle = $pagetitle . " - " . htmlspecialchars($system_url);
} else {
	$tabtitle = htmlspecialchars($system_url) . " - " . $pagetitle;
}

$cssfile = "/css/pfSense.css";

if (isset($user_settings['webgui']['webguicss'])) {
	if (file_exists("/usr/local/www/css/" . $user_settings['webgui']['webguicss'])) {
		$cssfile = "/css/" . $user_settings['webgui']['webguicss'];
	}
}

// set default columns to two if unset
if (!isset($config['system']['webgui']['dashboardcolumns'])) {
	$config['system']['webgui']['dashboardcolumns'] = 2;
}

?>
<!DOCTYPE html>
<html lang="en">

<head>
	<meta name="viewport" content="width=device-width, initial-scale=1">
	
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
	<link href="https://fonts.googleapis.com/css2?family=Fira+Sans:ital,wght@0,100;0,200;0,300;0,400;0,500;0,600;0,700;0,800;0,900;1,100;1,200;1,300;1,400;1,500;1,600;1,700;1,800;1,900&display=swap" rel="stylesheet">
	<link rel="apple-touch-icon-precomposed" href="/apple-touch/apple-touch-icon-iphone-60x60-precomposed.png">
	<link rel="apple-touch-icon-precomposed" sizes="60x60"
		href="/apple-touch/apple-touch-icon-ipad-76x76-precomposed.png">
	<link rel="apple-touch-icon-precomposed" sizes="114x114"
		href="/apple-touch/apple-touch-icon-iphone-retina-120x120-precomposed.png">
	<link rel="apple-touch-icon-precomposed" sizes="144x144"
		href="/apple-touch/apple-touch-icon-ipad-retina-152x152-precomposed.png">
	<link rel="icon" type="image/svg+xml" href="/favicon.svg" />
	<!-- <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"> -->
	<!-- <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"> -->
	<link rel="manifest" href="/manifest.json">
	<link rel="mask-icon" href="/safari-pinned-tab.svg" color="#5bbad5">
	<meta name="theme-color" content="#ffffff">

	<link rel="stylesheet"
		href="/vendor/font-awesome/css/all.min.css?v=<?= filemtime('/usr/local/www/vendor/font-awesome/css/all.min.css') ?>">
	<link rel="stylesheet"
		href="/vendor/font-awesome/css/v4-shims.css?v=<?= filemtime('/usr/local/www/vendor/font-awesome/css/v4-shims.css') ?>">
	<link rel="stylesheet"
		href="/vendor/sortable/sortable-theme-bootstrap.css?v=<?= filemtime('/usr/local/www/vendor/sortable/sortable-theme-bootstrap.css') ?>">
	<link rel="stylesheet"
		href="/vendor/jquery-treegrid/css/jquery.treegrid.css?v=<?= filemtime('/usr/local/www/vendor/jquery-treegrid/css/jquery.treegrid.css') ?>">
	<link rel="stylesheet" href="<?= $cssfile ?>?v=<?= filemtime('/usr/local/www/' . $cssfile) ?>" />

	<title>
		<?= $tabtitle ?>
	</title>
	<script type="text/javascript">
		//<![CDATA[
		var events = events || [];
		var newSeperator = false;
		//]]>
	</script>
</head>

<?php

/* Determine automated help URL. Should output the page name and parameters
separately */
$uri_split = "";
preg_match("/\/(.*)\?(.*)/", $_SERVER["REQUEST_URI"], $uri_split);

/* If there was no match, there were no parameters, just grab the filename
Otherwise, use the matched filename from above. */
if (empty($uri_split[0])) {
	$pagename = ltrim($_SERVER["REQUEST_URI"], '/');
} else {
	$pagename = $uri_split[1];
}

/* If the page name is still empty, the user must have requested / (index.php) */
if (empty($pagename)) {
	$pagename = "index.php";
}

/* If the filename is pkg_edit.php or wizard.php, reparse looking
for the .xml filename */
if (($pagename == "pkg.php") || ($pagename == "pkg_edit.php") || ($pagename == "wizard.php")) {
	$param_split = explode('&', $uri_split[2]);
	foreach ($param_split as $param) {
		if (substr($param, 0, 4) == "xml=") {
			$xmlfile = explode('=', $param);
			$pagename = $xmlfile[1];
		}
	}
} else if ($pagename == "status_logs.php") {
	$param_split = explode('&', $uri_split[2]);
	foreach ($param_split as $param) {
		if (substr($param, 0, 8) == "logfile=") {
			$logtype = explode('=', $param);
			$pagename .= '-' . $logtype[1];
		}
	}
}

// Build the full help URL.
$helpurl .= "{$g['help_base_url']}?page={$pagename}";

/*
 * Read files from $g['ext_menu_path']/*.xml and fill an array with menu info
 */
function read_ext_menu_path_data()
{
	global $g;

	$result = array();

	if (!is_dir($g['ext_menu_path'])) {
		return $result;
	}

	foreach (glob("{$g['ext_menu_path']}/*.xml") as $menu_xml) {
		$xml_data = parse_xml_config_pkg($menu_xml, "packagegui");
		if (empty($xml_data['menu'])) {
			continue;
		}
		foreach ($xml_data['menu'] as $menu) {
			$result[] = $menu;
		}
	}

	return $result;
}

// Create a menu entry of any installed packages in the specified category
// (Now reads the menu information from $config['installedpackages']['menu'] only)
function return_ext_menu($section)
{
	global $config, $ext_menu_path_data;

	$htmltext = "";
	$extarray = array();
	$ext_menu_entries = array();

	if ((!empty($config['installedpackages']['package'])) && (!empty($config['installedpackages']['menu']))) {
		foreach ($config['installedpackages']['menu'] as $menu) {
			if (isset($menu['name']) && ($menu['name'] != "AutoConfigBackup")) { // AutoConfigBackup was moved to a built-in function
				//			print('Name: ' . $menu['name'] . ', Pkg category: ' . $menu['category'] . ', Section: ' . $section . '<br />');
				if (isset($menu['section']) && ($menu['section'] == $section)) {
					$ext_menu_entries[] = $menu;
				}
			}
		}
	}

	foreach ($ext_menu_path_data as $menu) {
		if ($menu['section'] == $section) {
			$ext_menu_entries[] = $menu;
		}
	}

	foreach ($ext_menu_entries as $menu) {
		if ($menu['url'] != "") {
			$test_url = $menu['url'];
			$addresswithport = getenv("HTTP_HOST");
			$colonpos = strpos($addresswithport, ":");

			if ($colonpos !== false) {
				//my url is actually just the IP address of the pfsense box
				$myurl = substr($addresswithport, 0, $colonpos);
			} else {
				$myurl = $addresswithport;
			}
			$description = str_replace('$myurl', $myurl, $menu['url']);
		} else {
			$description = '/pkg.php?xml=' . $menu['configfile'];
			$test_url = $description;
		}

		if (isAllowedPage($test_url)) {
			$extarray[] = array($menu['name'], $description);
		}
	}

	return $extarray;
}

function output_menu($arrayitem, $target = null, $section = "")
{
	$output = "";

	foreach ($arrayitem as $item) {

		/* If the user has access to help pages, also show the full help menu. See #5909 */
		if (
			isAllowedPage($item[1]) || $item[1] == "/index.php?logout" ||
			(($section == "Help") && isAllowedPage("help.php")) ||
			(substr($item[1], 0, 8) == "https://")
		) {
			$attr = sprintf("href=\"%s\"", htmlentities($item[1]));

			if ($target) {
				$attr .= sprintf(" target=\"%s\"", htmlentities($target));
			}

			$class = "navlnk";

			if ($item['class']) {
				$class .= " {$item['class']}";
			}

			$attr .= sprintf(" class=\"%s\"", htmlentities($class));

			if ($item['style']) {
				$attr .= sprintf(" style=\"%s\"", htmlentities($item['style']));
			}


			if ($item[0] != '-DIVIDER-') {
				$output .= "<li>" . sprintf("<a style =\"color:white;\" %s %s>%s</a>", $attr, ($item[1] == "/index.php?logout") ? "usepost" : "", $item[0]) . "</li>\n";
			}

			// if ($item[0] == '-DIVIDER-') {
			// 	$output .= ' <li class="divider"></li>';
			// } else {
			// 	$output .= "<li>" . sprintf("<a %s %s>%s</a>", $attr, ($item[1] == "/index.php?logout") ? "usepost" : "", $item[0]) . "</li>\n";
			// }
		}
	}

	return $output;
}

$ext_menu_path_data = read_ext_menu_path_data();

// System
$system_menu = array();
$system_menu[] = array(gettext("Logout") . " (" . $_SESSION['Username'] . ")", "/index.php?logout");
$system_menu[] = array(gettext("Advanced"), "/system_advanced_admin.php");
$system_menu[] = array(gettext("Update"), "/pkg_mgr_install.php?id=firmware");
$system_menu[] = array(gettext("General Setup"), "/system.php");
$system_menu[] = array(gettext("High Avail. Sync"), "/system_hasync.php");
$system_menu[] = array(gettext("Package Manager"), "/pkg_mgr_installed.php");
$system_menu[] = array(gettext("Setup Wizard"), "/wizard.php?xml=setup_wizard.xml");
$system_menu[] = array(gettext("Routing"), "/system_gateways.php");
$system_menu[] = array(gettext("Cert. Manager"), "/system_camanager.php");
if (strpos($g['product_label'], 'Plus') === false) {
	$system_menu[] = array(gettext("Register"), "/system_register.php");
}

if (!isAllowedPage("system_usermanager.php")) {
	$system_menu[] = array(gettext("User Manager"), "/system_usermanager_passwordmg.php");
} else {
	$system_menu[] = array(gettext("User Manager"), "/system_usermanager.php");
}

if ($user_settings['customsettings'] && isAllowedPage("system_user_settings.php")) {
	$system_menu[] = array(gettext("User Settings"), "/system_user_settings.php");
}

$system_menu = msort(array_merge($system_menu, return_ext_menu("System")), 0);

// Interfaces
// NOTE:
// Now that menus are sorted, adding a DIVIDER must be done after the sorting so an array is formed of the
// items above the divider and another for below it. These are then sorted and combined with the divider
$interfaces_menu = array();
$interfaces_top = array();
$interfaces_bottom = array();

if (!isset($config['system']['webgui']['noassigninterfaces'])) {
	$interfaces_top[] = array(gettext("Assignments"), "/interfaces_assign.php");
	$div = true;
}

$platform = system_identify_specific_platform();

if ($platform['name'] == "uFW") {
	$interfaces_top[] = array(gettext("Switches"), "/switch_system.php");
}

$opts = get_configured_interface_with_descr(true);

foreach ($opts as $oif => $odescr) {
	if (!isset($config['interfaces'][$oif]['ovpn'])) {
		$interfaces_bottom[] = array(htmlspecialchars($odescr), "/interfaces.php?if={$oif}");
	}
}

$interfaces_bottom = array_merge($interfaces_bottom, return_ext_menu("Interfaces"));

if ($user_settings['webgui']['interfacessort']) {
	$interfaces_bottom = msort($interfaces_bottom, 0);
}

// Combine the top section, the divider and the bottom section of this menu
$interfaces_menu = array_merge($interfaces_top, [array(0 => "-DIVIDER-")], $interfaces_bottom);

// Firewall
$firewall_menu = array();
$firewall_menu[] = array(gettext("Aliases"), "/firewall_aliases.php");
$firewall_menu[] = array(gettext("NAT"), "/firewall_nat.php");
$firewall_menu[] = array(gettext("Rules"), "/firewall_rules.php");
$firewall_menu[] = array(gettext("Schedules"), "/firewall_schedule.php");
$firewall_menu[] = array(gettext("Traffic Shaper"), "/firewall_shaper.php");
$firewall_menu[] = array(gettext("Virtual IPs"), "/firewall_virtual_ip.php");
$firewall_menu = msort(array_merge($firewall_menu, return_ext_menu("Firewall")), 0);

// Services
$services_menu = array();
$services_menu[] = array(gettext("Auto Config Backup"), "/services_acb.php");
$services_menu[] = array(gettext("Captive Portal"), "/services_captiveportal.php");
$services_menu[] = array(gettext("DNS Forwarder"), "/services_dnsmasq.php");
$services_menu[] = array(gettext("DNS Resolver"), "/services_unbound.php");
$services_menu[] = array(gettext("DHCP Relay"), "/services_dhcp_relay.php");
$services_menu[] = array(gettext("DHCPv6 Relay"), "/services_dhcpv6_relay.php");

if ($g['services_dhcp_server_enable']) {
	$services_menu[] = array(gettext("DHCP Server"), "/services_dhcp.php");
	$services_menu[] = array(htmlspecialchars(gettext("DHCPv6 Server & RA")), "/services_dhcpv6.php");
}

$services_menu[] = array(gettext("Dynamic DNS"), "/services_dyndns.php");
$services_menu[] = array(gettext("IGMP Proxy"), "/services_igmpproxy.php");
$services_menu[] = array(gettext("NTP"), "/services_ntpd.php");
$services_menu[] = array(gettext("PPPoE Server"), "/services_pppoe.php");
$services_menu[] = array(gettext("SNMP"), "/services_snmp.php");

if (count($config['interfaces']) > 1) {
	/* no use for UPnP in single-interface deployments
	remove to reduce user confusion
	*/
	$services_menu[] = array(gettext("UPnP &amp; NAT-PMP"), "/pkg_edit.php?xml=miniupnpd.xml");
}

$services_menu[] = array(gettext("Wake-on-LAN"), "/services_wol.php");
$services_menu = msort(array_merge($services_menu, return_ext_menu("Services")), 0);

// VPN
$vpn_menu = array();
$vpn_menu[] = array(gettext("IPsec"), "/vpn_ipsec.php");
$vpn_menu[] = array(gettext("OpenVPN"), "/vpn_openvpn_server.php");
//$vpn_menu[] = array(gettext("PPTP"), "/vpn_pptp.php");
$vpn_menu[] = array(gettext("L2TP"), "/vpn_l2tp.php");
$vpn_menu = msort(array_merge($vpn_menu, return_ext_menu("VPN")), 0);

// Status
$status_menu = array();
$status_menu[] = array(gettext("Captive Portal"), "/status_captiveportal.php");
$status_menu[] = array(gettext("CARP (failover)"), "/status_carp.php");
$status_menu[] = array(gettext("Dashboard"), "/index.php");
$status_menu[] = array(gettext("Gateways"), "/status_gateways.php");
$status_menu[] = array(gettext("DHCP Leases"), "/status_dhcp_leases.php");
$status_menu[] = array(gettext("DHCPv6 Leases"), "/status_dhcpv6_leases.php");
$status_menu[] = array(gettext("DNS Resolver"), "/status_unbound.php");
$status_menu[] = array(gettext("Filter Reload"), "/status_filter_reload.php?user=true");
$status_menu[] = array(gettext("Interfaces"), "/status_interfaces.php");
$status_menu[] = array(gettext("IPsec"), "/status_ipsec.php");
$status_menu[] = array(gettext("NTP"), "/status_ntpd.php");
$status_menu[] = array(gettext("OpenVPN"), "/status_openvpn.php");
$status_menu[] = array(gettext("Queues"), "/status_queues.php");
$status_menu[] = array(gettext("Services"), "/status_services.php");
$status_menu[] = array(gettext("System Logs"), "/status_logs.php");
$status_menu[] = array(gettext("Traffic Graph"), "/status_graph.php");

if (count($config['interfaces']) > 1) {
	$status_menu[] = array(gettext("UPnP &amp; NAT-PMP"), "/status_upnp.php");
}

$wifdescrs = array();
$ifentries = get_configured_interface_with_descr();
foreach ($ifentries as $ent => $entdesc) {
	if (
		is_array($config['interfaces'][$ent]['wireless']) &&
		preg_match($g['wireless_regex'], $config['interfaces'][$ent]['if'])
	) {
		$wifdescrs[$ent] = $entdesc;
	}
}

if (count($wifdescrs) > 0) {
	$status_menu[] = array(gettext("Wireless"), "/status_wireless.php");
}

$status_menu = msort(array_merge($status_menu, return_ext_menu("Status")), 0);

// Diagnostics
$diagnostics_menu = array();
$diagnostics_menu[] = array(gettext("ARP Table"), "/diag_arp.php");
$diagnostics_menu[] = array(gettext("Authentication"), "/diag_authentication.php");
$diagnostics_menu[] = array(htmlspecialchars(gettext("Backup & Restore")), "/diag_backup.php");
$diagnostics_menu[] = array(gettext("Command Prompt"), "/diag_command.php");
$diagnostics_menu[] = array(gettext("DNS Lookup"), "/diag_dns.php");
$diagnostics_menu[] = array(gettext("Edit File"), "/diag_edit.php");
$diagnostics_menu[] = array(gettext("Factory Defaults"), "/diag_defaults.php");

if (file_exists("/var/run/gmirror_active")) {
	$diagnostics_menu[] = array(gettext("GEOM Mirrors"), "/diag_gmirror.php");
}

$diagnostics_menu[] = array(gettext("Halt System"), "/diag_halt.php");
$diagnostics_menu[] = array(gettext("Limiter Info"), "/diag_limiter_info.php");
$diagnostics_menu[] = array(gettext("NDP Table"), "/diag_ndp.php");
$diagnostics_menu[] = array(gettext("Tables"), "/diag_tables.php");
$diagnostics_menu[] = array(gettext("Ping"), "/diag_ping.php");
$diagnostics_menu[] = array(gettext("Test Port"), "/diag_testport.php");
$diagnostics_menu[] = array(gettext("pfInfo"), "/diag_pf_info.php");
$diagnostics_menu[] = array(gettext("pfTop"), "/diag_pftop.php");
$diagnostics_menu[] = array(gettext("Reboot"), "/diag_reboot.php");
$diagnostics_menu[] = array(gettext("Routes"), "/diag_routes.php");
$diagnostics_menu[] = array(gettext("S.M.A.R.T. Status"), "/diag_smart.php");
$diagnostics_menu[] = array(gettext("Sockets"), "/diag_sockets.php");
$diagnostics_menu[] = array(gettext("States"), "/diag_dump_states.php");
$diagnostics_menu[] = array(gettext("States Summary"), "/diag_states_summary.php");
$diagnostics_menu[] = array(gettext("System Activity"), "/diag_system_activity.php");
$diagnostics_menu[] = array(gettext("Traceroute"), "/diag_traceroute.php");
$diagnostics_menu[] = array(gettext("Packet Capture"), "/diag_packet_capture.php");

$diagnostics_menu = msort(array_merge($diagnostics_menu, return_ext_menu("Diagnostics")), 0);

if (!$g['disablehelpmenu']) {
	$help_menu = array();
	$help_menu[] = array(gettext("About this Page"), $helpurl);
	if ($g['product_name'] == "pfSense") {
		$help_menu[] = array(gettext("Bug Database"), "https://redirects.netgate.com/issues");
	}

	$help_menu[] = array(gettext("User Forum"), "https://redirects.netgate.com/forum");
	$help_menu[] = array(gettext("Documentation"), "https://redirects.netgate.com/docs");
	$help_menu[] = array(gettext("Paid Support"), "https://redirects.netgate.com/support");
	$help_menu[] = array(gettext("pfSense Book"), "https://redirects.netgate.com/book");
	$help_menu[] = array(gettext("FreeBSD Handbook"), "https://redirects.netgate.com/fbsdhandbook");
	$help_menu[] = array(gettext("User survey"), "https://redirects.netgate.com/survey_1");
	if (strpos($g['product_label'], 'Plus') === false) {
		$help_menu[] = array(gettext("Upgrade to pfSense Plus"), "https://redirects.netgate.com/upgrade");
	}
	$help_menu = msort(array_merge($help_menu, return_ext_menu("Help")), 0);
}

$menuclass = "static";

if ($user_settings['webgui']['webguifixedmenu'] == "fixed") {
	$menuclass = "fixed";
}

$numColumns = (int) $user_settings['webgui']['dashboardcolumns'];

if (($pagename === "index.php") && ($numColumns > 2)) {
	$columnsContainer = 'style="max-width: ' . 585 * $numColumns . 'px;width: 100%"';
}

$display_notices = false;
$allow_clear_notices = false;

if (are_notices_pending()) {
	// Evaluate user privs to determine if notices should be displayed, and if the user can clear them.
	$user_entry = getUserEntry($_SESSION['Username']);
	if (isAdminUID($_SESSION['Username']) || userHasPrivilege($user_entry, "user-view-clear-notices") || userHasPrivilege($user_entry, "page-all")) {
		$display_notices = true;
		$allow_clear_notices = true;
	} elseif (userHasPrivilege($user_entry, "user-view-notices")) {
		$display_notices = true;
	}
}
?>

<body id="<?= $numColumns ?>">
	<!-- PTA MAIN -->
	<div id="pta-main">
		<!-- PTA MENU -->
		<nav id="pta-menu" class="pta-menu d-flex navbar-inverse">
		<div> <a class="navbar-brand" href="/">
                      <svg
                        width="150px"
                        height="50px"
                        viewBox="-4.724 8.504 194.804 58.97"
                        fill="none"
                        xmlns="http://www.w3.org/2000/svg"
                      >
                        <title id="pfsense-logo-svg">PSS</title>
                        <path
                          d="M 22.039 24.419 C 22.004 23.311 21.556 22.452 20.601 21.892 C 19.641 21.332 18.581 21.189 17.503 21.183 C 12.548 21.171 7.588 21.177 2.634 21.177 C 2.439 21.177 2.239 21.183 2.045 21.177 C 1.903 21.171 1.838 21.218 1.803 21.367 C 1.691 21.85 1.556 22.327 1.432 22.804 C 1.226 23.597 1.019 24.39 0.813 25.206 L 9.939 25.206 C 9.939 25.224 9.939 25.242 9.939 25.26 C 9.88 25.284 9.815 25.314 9.756 25.337 C 7.606 26.16 5.45 26.983 3.299 27.799 C 3.146 27.859 3.04 27.931 2.993 28.109 C 1.844 32.419 0.684 36.735 -0.477 41.045 C -0.536 41.272 -0.506 41.319 -0.271 41.319 C 1.85 41.307 3.971 41.307 6.092 41.319 C 6.304 41.319 6.369 41.248 6.422 41.057 C 6.922 39.155 7.441 37.254 7.941 35.346 C 8.012 35.084 8.118 35.006 8.377 35.018 C 9.349 35.048 10.322 35.06 11.294 35.054 C 12.684 35.042 14.08 35.149 15.464 34.941 C 17.88 34.571 19.629 33.331 20.49 30.965 C 20.979 29.624 21.244 28.217 21.621 26.846 C 21.833 26.047 22.062 25.26 22.039 24.419 Z M 14.239 28.092 C 14.109 28.616 13.992 29.153 13.774 29.653 C 13.403 30.512 12.766 30.977 11.836 30.995 C 10.993 31.012 10.151 31.001 9.302 31.007 C 9.137 31.007 9.102 30.971 9.149 30.804 C 9.638 29.01 10.121 27.215 10.598 25.415 C 10.646 25.248 10.722 25.2 10.887 25.206 C 11.317 25.218 11.753 25.212 12.271 25.212 C 12.707 25.236 13.238 25.159 13.762 25.272 C 14.333 25.397 14.598 25.725 14.593 26.321 C 14.593 26.935 14.386 27.507 14.239 28.092 Z"
                          fill="#3157A1"
                        ></path>
                        <path
                          d="M 38.828 24.916 C 38.15 24.684 37.455 24.595 36.742 24.589 C 33.767 24.583 30.792 24.565 27.817 24.583 C 26.374 24.595 25.29 25.274 24.601 26.568 C 24.383 26.979 24.229 27.42 24.082 27.861 C 24.006 28.094 24.053 28.159 24.306 28.159 C 26.044 28.147 27.782 28.153 29.52 28.153 C 29.643 28.153 29.773 28.124 29.914 28.207 C 29.82 28.249 29.743 28.285 29.661 28.314 C 27.676 29.107 25.69 29.894 23.699 30.681 C 23.534 30.746 23.452 30.824 23.405 31.003 C 23.063 32.272 22.704 33.542 22.362 34.812 C 22.132 35.664 21.844 36.499 21.749 37.381 C 21.584 38.842 22.25 39.927 23.623 40.409 C 24.277 40.642 24.96 40.749 25.643 40.749 C 29.502 40.767 33.361 40.761 37.213 40.773 C 37.414 40.773 37.455 40.672 37.496 40.523 C 37.767 39.468 38.032 38.412 38.315 37.363 C 38.374 37.137 38.321 37.077 38.091 37.077 C 35.964 37.083 33.832 37.083 31.699 37.083 C 31.004 37.083 30.303 37.089 29.608 37.083 C 28.766 37.071 28.418 36.642 28.536 35.801 C 28.589 35.414 28.724 35.056 28.842 34.687 C 28.907 34.496 29.001 34.454 29.178 34.454 C 30.433 34.478 31.688 34.46 32.942 34.49 C 34.827 34.532 36.713 34.496 38.598 34.508 C 38.839 34.508 38.934 34.442 38.992 34.204 C 39.37 32.755 39.77 31.319 40.147 29.87 C 40.359 29.059 40.618 28.255 40.601 27.396 C 40.583 26.15 39.994 25.322 38.828 24.916 Z M 33.838 29.399 C 33.773 29.924 33.608 30.419 33.496 30.931 C 33.473 31.039 33.425 31.104 33.302 31.104 C 32.147 31.098 30.986 31.104 29.802 31.104 C 30.05 30.162 30.285 29.244 30.521 28.326 C 30.551 28.207 30.621 28.183 30.733 28.183 C 31.44 28.189 32.147 28.171 32.848 28.201 C 33.579 28.225 33.932 28.672 33.838 29.399 Z"
                          fill="#3157A1"
                        ></path>
                        <path
                          d="M 40.041 40.773 C 40.624 38.526 41.202 36.326 41.773 34.12 C 42.038 33.107 42.309 32.094 42.568 31.074 C 42.616 30.895 42.698 30.794 42.869 30.729 C 44.831 29.948 46.786 29.161 48.742 28.38 C 48.86 28.332 48.978 28.285 49.119 28.225 C 48.996 28.13 48.878 28.165 48.772 28.165 C 47.034 28.165 45.296 28.165 43.558 28.171 C 43.364 28.171 43.328 28.135 43.387 27.939 C 43.664 26.907 43.935 25.87 44.194 24.833 C 44.241 24.642 44.33 24.589 44.518 24.589 C 48.306 24.589 52.094 24.583 55.888 24.589 C 56.524 24.589 57.161 24.66 57.779 24.845 C 59.317 25.304 60.006 26.395 59.764 28.01 C 59.588 29.197 59.205 30.335 58.898 31.498 C 58.109 34.496 57.296 37.483 56.501 40.475 C 56.442 40.702 56.359 40.779 56.112 40.779 C 54.186 40.767 52.265 40.767 50.339 40.779 C 50.103 40.779 50.044 40.743 50.115 40.487 C 51.057 36.988 51.988 33.489 52.919 29.989 C 52.99 29.721 53.031 29.447 53.025 29.167 C 53.013 28.624 52.795 28.344 52.259 28.243 C 51.735 28.141 51.205 28.219 50.675 28.195 C 50.374 28.183 50.003 28.1 49.785 28.237 C 49.555 28.38 49.585 28.785 49.502 29.077 C 48.489 32.863 47.476 36.648 46.48 40.439 C 46.403 40.719 46.297 40.779 46.026 40.779 C 44.153 40.767 42.274 40.773 40.4 40.773 C 40.289 40.773 40.183 40.773 40.041 40.773 Z"
                          fill="#3157A1"
                        ></path>
                        <path
                          d="M 81.055 29.739 C 80.548 31.551 80.048 33.334 79.553 35.11 C 79.447 35.486 79.358 35.861 79.376 36.249 C 79.4 36.749 79.641 37.059 80.136 37.119 C 80.955 37.22 81.785 37.155 82.604 37.155 C 82.805 37.155 82.769 36.982 82.799 36.874 C 83.223 35.277 83.641 33.679 84.053 32.076 C 84.266 31.259 84.466 30.442 84.672 29.626 C 84.684 29.566 84.696 29.507 84.701 29.447 C 84.784 28.577 84.436 28.177 83.57 28.183 C 80.972 28.189 78.369 28.207 75.771 28.213 C 75.553 28.213 75.341 28.201 75.123 28.207 C 74.952 28.213 74.893 28.165 74.946 27.986 C 75.252 26.913 75.559 25.834 75.859 24.756 C 75.894 24.63 75.941 24.571 76.077 24.577 C 76.212 24.583 76.354 24.571 76.489 24.571 C 80.095 24.571 83.694 24.565 87.299 24.571 C 88.107 24.571 88.896 24.72 89.644 25.06 C 91.07 25.703 91.741 26.955 91.388 28.499 C 91.011 30.126 90.557 31.736 90.127 33.357 C 89.503 35.718 88.866 38.085 88.248 40.445 C 88.183 40.69 88.089 40.761 87.836 40.761 C 84.183 40.749 80.531 40.755 76.878 40.755 C 76.13 40.755 75.388 40.707 74.663 40.499 C 73.131 40.064 72.436 38.967 72.66 37.375 C 72.825 36.213 73.249 35.116 73.55 33.989 C 73.662 33.566 73.797 33.155 73.915 32.731 C 73.95 32.594 74.009 32.511 74.145 32.457 C 76.413 31.551 78.681 30.645 80.949 29.739 C 80.961 29.745 80.99 29.745 81.055 29.739 Z"
                          fill="#3157A1"
                        ></path>
                        <path
                          d="M 71.977 19.867 C 71.594 21.34 71.217 22.794 70.834 24.249 C 70.74 24.601 70.74 24.595 71.111 24.589 C 71.977 24.583 72.837 24.595 73.703 24.583 C 73.891 24.583 73.891 24.654 73.856 24.791 C 73.573 25.864 73.29 26.937 73.019 28.01 C 72.978 28.177 72.902 28.225 72.737 28.225 C 71.853 28.219 70.969 28.225 70.086 28.219 C 69.879 28.219 69.785 28.273 69.726 28.493 C 69.09 30.967 68.436 33.435 67.794 35.903 C 67.588 36.696 67.859 37.077 68.66 37.143 C 69.296 37.196 69.932 37.161 70.575 37.167 C 70.781 37.167 70.869 37.184 70.804 37.435 C 70.522 38.472 70.251 39.521 69.985 40.564 C 69.944 40.719 69.885 40.773 69.732 40.779 C 67.605 40.857 65.473 40.815 63.352 40.642 C 62.981 40.612 62.616 40.517 62.268 40.392 C 61.373 40.07 60.925 39.45 60.907 38.484 C 60.89 37.62 61.119 36.797 61.337 35.974 C 62.003 33.489 62.669 31.003 63.352 28.523 C 63.423 28.255 63.376 28.207 63.116 28.213 C 62.362 28.231 61.602 28.219 60.848 28.231 C 60.689 28.231 60.595 28.237 60.654 28.01 C 60.943 26.943 61.22 25.87 61.491 24.791 C 61.538 24.607 61.632 24.589 61.785 24.589 C 62.539 24.595 63.299 24.583 64.053 24.595 C 64.277 24.595 64.395 24.547 64.454 24.297 C 64.807 22.884 65.19 21.483 65.549 20.07 C 65.591 19.915 65.644 19.844 65.82 19.844 C 67.841 19.85 69.868 19.85 71.888 19.85 C 71.9 19.85 71.918 19.855 71.977 19.867 Z"
                          fill="#3157A1"
                        ></path>
                        <path
                          d="M 177.38 32.38 C 178.517 30.961 179.619 29.584 180.726 28.201 C 181.586 27.128 182.446 26.061 183.306 24.988 C 183.371 24.905 183.43 24.845 183.548 24.845 C 184.125 24.851 184.703 24.845 185.309 24.845 C 185.239 25.083 185.062 25.22 184.932 25.375 C 182.57 28.267 180.208 31.158 177.845 34.049 C 177.669 34.269 177.616 34.526 177.551 34.782 C 177.074 36.785 176.597 38.788 176.131 40.791 C 176.096 40.952 176.025 41.017 175.848 41.012 C 175.465 41 175.082 40.994 174.7 41.012 C 174.452 41.023 174.429 40.94 174.482 40.731 C 174.812 39.396 175.13 38.055 175.448 36.719 C 175.63 35.968 175.807 35.217 175.99 34.466 C 176.025 34.317 176.007 34.174 175.96 34.031 C 175.236 31.778 174.517 29.524 173.792 27.271 C 173.557 26.538 173.321 25.805 173.027 25.066 C 172.938 25.429 172.838 25.787 172.767 26.156 C 172.72 26.389 172.632 26.472 172.378 26.472 C 171.012 26.454 169.651 26.466 168.284 26.46 C 168.107 26.46 168.025 26.49 167.972 26.699 C 167.018 30.735 166.051 34.764 165.091 38.794 C 164.932 39.456 164.761 40.117 164.608 40.785 C 164.567 40.97 164.484 41.017 164.308 41.012 C 163.925 41 163.542 41 163.159 41.012 C 162.964 41.017 162.911 40.982 162.964 40.761 C 163.318 39.319 163.66 37.876 164.001 36.433 C 164.779 33.191 165.551 29.948 166.334 26.711 C 166.381 26.52 166.34 26.46 166.134 26.46 C 164.767 26.472 163.406 26.46 162.04 26.472 C 161.774 26.472 161.727 26.413 161.798 26.162 C 161.898 25.823 161.975 25.471 162.045 25.125 C 162.087 24.934 162.152 24.851 162.369 24.851 C 166.464 24.857 170.558 24.857 174.652 24.851 C 174.847 24.851 174.941 24.905 175.006 25.101 C 175.742 27.408 176.49 29.715 177.239 32.022 C 177.286 32.123 177.327 32.225 177.38 32.38 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 155.236 25.322 C 154.623 24.97 153.939 24.863 153.25 24.857 C 150.906 24.839 148.561 24.851 146.216 24.839 C 145.992 24.839 145.945 24.934 145.898 25.125 C 144.944 29.149 143.978 33.167 143.017 37.19 C 142.734 38.371 142.452 39.557 142.163 40.737 C 142.11 40.952 142.157 41.023 142.375 41.012 C 142.746 41 143.123 40.994 143.494 41.012 C 143.707 41.023 143.771 40.94 143.818 40.743 C 144.29 38.752 144.773 36.761 145.244 34.77 C 145.291 34.579 145.356 34.544 145.533 34.55 C 146.063 34.568 146.593 34.556 147.123 34.556 C 148.39 34.562 149.657 34.544 150.923 34.573 C 151.748 34.591 152.408 34.979 152.42 35.962 C 152.425 36.344 152.39 36.714 152.308 37.083 C 152.025 38.317 151.736 39.551 151.436 40.779 C 151.389 40.976 151.424 41.023 151.612 41.017 C 151.995 41.006 152.378 41 152.761 41.017 C 152.973 41.029 153.05 40.946 153.097 40.749 C 153.244 40.093 153.415 39.444 153.557 38.782 C 153.769 37.787 154.122 36.821 154.187 35.796 C 154.252 34.788 153.875 34.061 152.802 33.852 C 152.72 33.834 152.638 33.805 152.49 33.763 C 153.869 33.56 154.794 32.839 155.306 31.611 C 155.872 30.252 156.237 28.833 156.255 27.349 C 156.255 26.514 156.013 25.769 155.236 25.322 Z M 154.605 27.802 C 154.487 29.083 154.193 30.323 153.639 31.486 C 153.221 32.368 152.449 32.737 151.53 32.845 C 150.287 32.988 149.038 32.886 147.795 32.91 C 147.188 32.922 146.576 32.898 145.969 32.916 C 145.733 32.922 145.692 32.857 145.745 32.636 C 146.122 31.092 146.493 29.542 146.864 27.992 C 146.958 27.587 147.065 27.188 147.147 26.776 C 147.194 26.55 147.271 26.454 147.53 26.46 C 148.396 26.478 149.256 26.466 150.122 26.466 L 150.122 26.472 C 150.964 26.472 151.813 26.466 152.655 26.478 C 153.056 26.484 153.456 26.532 153.839 26.669 C 154.358 26.848 154.658 27.241 154.605 27.802 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 104.69 29.268 C 104.755 28.827 104.831 28.416 104.878 28.004 C 105.002 26.872 104.596 26.52 103.747 26.425 C 102.457 26.282 101.167 26.305 99.871 26.443 C 98.646 26.568 97.974 27.277 97.674 28.428 C 97.515 29.042 97.373 29.656 97.314 30.293 C 97.232 31.17 97.721 31.718 98.687 31.831 C 99.523 31.927 100.372 31.885 101.214 31.951 C 102.051 32.016 102.899 32.028 103.718 32.249 C 104.649 32.499 105.126 33.071 105.144 34.037 C 105.173 35.843 104.714 37.53 103.853 39.104 C 103.205 40.29 102.127 40.851 100.831 41.011 C 99.358 41.196 97.874 41.178 96.395 41.065 C 95.9 41.029 95.406 40.934 94.94 40.737 C 94.157 40.409 93.774 39.795 93.709 38.967 C 93.632 37.989 93.862 37.059 94.121 36.129 C 94.163 35.992 94.239 35.945 94.381 35.951 C 94.775 35.956 95.164 35.968 95.559 35.951 C 95.783 35.939 95.824 36.004 95.771 36.213 C 95.624 36.767 95.506 37.328 95.464 37.906 C 95.394 38.913 95.753 39.384 96.749 39.456 C 98.098 39.551 99.453 39.575 100.802 39.42 C 101.68 39.319 102.304 38.883 102.669 38.079 C 103.076 37.196 103.323 36.272 103.417 35.307 C 103.529 34.222 103.141 33.775 102.074 33.691 C 100.873 33.602 99.671 33.53 98.469 33.441 C 98.039 33.411 97.609 33.363 97.191 33.256 C 95.895 32.916 95.494 32.159 95.559 30.937 C 95.635 29.518 96.024 28.165 96.702 26.913 C 97.356 25.709 98.392 25.071 99.712 24.869 C 101.085 24.654 102.469 24.702 103.848 24.756 C 104.207 24.773 104.566 24.827 104.92 24.916 C 106.292 25.25 106.693 26.103 106.652 27.361 C 106.634 27.933 106.51 28.499 106.375 29.059 C 106.334 29.214 106.275 29.286 106.098 29.286 C 105.627 29.256 105.161 29.268 104.69 29.268 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 121.008 41.131 C 120.413 41.095 119.683 41.184 118.946 41.077 C 117.621 40.886 116.714 39.909 116.555 38.561 C 116.425 37.459 116.661 36.415 116.914 35.366 C 117.391 33.393 117.857 31.42 118.334 29.453 C 118.499 28.779 118.752 28.13 119.076 27.516 C 119.806 26.144 120.938 25.322 122.428 24.982 C 123.247 24.797 124.072 24.72 124.908 24.732 C 125.898 24.744 126.888 24.66 127.871 24.791 C 129.173 24.964 129.904 25.769 129.921 27.11 C 129.933 27.998 129.745 28.863 129.533 29.715 C 129.497 29.858 129.427 29.882 129.303 29.882 C 128.89 29.876 128.478 29.876 128.066 29.882 C 127.877 29.888 127.865 29.817 127.901 29.644 C 128.007 29.185 128.125 28.72 128.172 28.249 C 128.307 27.003 127.977 26.478 126.758 26.383 C 125.362 26.282 123.954 26.258 122.569 26.556 C 121.303 26.83 120.537 27.647 120.148 28.857 C 119.73 30.156 119.477 31.504 119.153 32.827 C 118.834 34.132 118.487 35.432 118.292 36.767 C 118.222 37.25 118.186 37.733 118.263 38.216 C 118.375 38.889 118.84 39.372 119.63 39.438 C 121.173 39.563 122.723 39.611 124.26 39.342 C 125.073 39.199 125.639 38.722 125.986 37.965 C 126.31 37.268 126.434 36.511 126.611 35.772 C 126.664 35.551 126.746 35.438 126.982 35.48 C 127.365 35.545 127.748 35.462 128.125 35.48 C 128.213 35.486 128.343 35.45 128.295 35.623 C 128.025 36.63 127.865 37.673 127.4 38.627 C 126.705 40.052 125.544 40.779 124.03 41 C 123.094 41.155 122.122 41.125 121.008 41.131 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 134.104 41.125 C 133.014 41.137 132.036 41.143 131.076 40.892 C 129.851 40.57 129.244 39.682 129.268 38.395 C 129.285 36.994 129.727 35.688 130.033 34.347 C 130.634 31.724 131.276 29.113 131.895 26.496 C 132.007 26.013 132.131 25.536 132.243 25.054 C 132.278 24.911 132.343 24.839 132.502 24.845 C 132.885 24.857 133.268 24.863 133.651 24.845 C 133.91 24.833 133.933 24.922 133.88 25.149 C 133.598 26.299 133.315 27.456 133.044 28.612 C 132.372 31.432 131.707 34.252 131.035 37.071 C 130.947 37.453 130.917 37.834 130.941 38.222 C 130.97 38.669 131.182 38.985 131.583 39.175 C 131.96 39.354 132.372 39.426 132.779 39.45 C 134.092 39.533 135.406 39.569 136.714 39.408 C 137.745 39.283 138.446 38.728 138.847 37.769 C 139.212 36.904 139.371 35.986 139.589 35.086 C 140.237 32.404 140.873 29.721 141.515 27.039 C 141.674 26.383 141.833 25.733 141.981 25.077 C 142.016 24.916 142.075 24.845 142.252 24.851 C 142.646 24.863 143.035 24.863 143.43 24.851 C 143.642 24.845 143.665 24.905 143.618 25.107 C 143.153 27.009 142.705 28.916 142.252 30.818 C 141.763 32.869 141.285 34.919 140.767 36.964 C 140.502 38.025 140.054 39.003 139.265 39.784 C 138.499 40.541 137.545 40.862 136.52 41.011 C 135.689 41.125 134.846 41.155 134.104 41.125 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 105.161 40.988 C 105.338 40.237 105.503 39.497 105.68 38.758 C 106.54 35.14 107.406 31.527 108.272 27.909 C 108.496 26.967 108.737 26.025 108.955 25.083 C 108.996 24.893 109.079 24.851 109.256 24.851 C 112.16 24.857 115.064 24.857 117.974 24.851 C 118.216 24.851 118.257 24.934 118.198 25.149 C 118.104 25.501 118.004 25.858 117.945 26.216 C 117.91 26.437 117.803 26.478 117.609 26.472 C 115.294 26.466 112.979 26.472 110.658 26.46 C 110.357 26.46 110.251 26.538 110.18 26.842 C 109.815 28.451 109.426 30.055 109.026 31.658 C 108.967 31.903 109.014 31.957 109.256 31.951 C 111.476 31.945 113.692 31.951 115.912 31.939 C 116.178 31.939 116.266 31.98 116.172 32.267 C 116.066 32.577 116.001 32.898 115.942 33.214 C 115.907 33.405 115.818 33.447 115.641 33.453 C 114.375 33.506 113.108 33.465 111.842 33.477 C 110.84 33.483 109.839 33.483 108.843 33.471 C 108.643 33.471 108.578 33.554 108.531 33.739 C 108.107 35.539 107.677 37.339 107.241 39.134 C 107.194 39.319 107.253 39.372 107.412 39.378 C 107.471 39.378 107.529 39.378 107.588 39.378 C 109.915 39.378 112.242 39.384 114.569 39.372 C 114.758 39.372 114.787 39.414 114.746 39.581 C 114.646 39.98 114.546 40.386 114.457 40.791 C 114.422 40.94 114.363 41.006 114.204 41.006 C 111.241 41 108.278 41 105.309 41 C 105.261 41.006 105.226 41 105.161 40.988 Z"
                          fill="#999999"
                        ></path>
                        <path
                          d="M 156.031 41.012 C 155.866 41.012 155.695 40.994 155.53 41.017 C 155.265 41.047 155.2 40.976 155.265 40.702 C 155.636 39.205 155.99 37.703 156.349 36.201 C 157.233 32.505 158.11 28.815 158.988 25.119 C 159.035 24.928 159.1 24.833 159.318 24.845 C 159.701 24.869 160.084 24.857 160.467 24.845 C 160.643 24.839 160.702 24.875 160.649 25.077 C 160.184 26.991 159.736 28.904 159.277 30.818 C 158.493 34.097 157.704 37.375 156.932 40.654 C 156.867 40.916 156.791 41.059 156.502 41.012 C 156.349 40.988 156.19 41.012 156.031 41.012 Z"
                          fill="#999999"
                        ></path>
                      </svg>

                    </a>
		</div>
		<div class="menu-update">
			<div class=" menu-item-update collapse " id="pf-navbar">
				<ul class="nav navbar-nav d-flex d-column">
				<?php
				if ($user_settings['webgui']['webguihostnamemenu'] == 'hostonly') {
					$help_menu_title = htmlspecialchars($config['system']['hostname']);
				} elseif ($user_settings['webgui']['webguihostnamemenu'] == 'fqdn') {
					$help_menu_title = htmlspecialchars($system_url);
				} else {
					$help_menu_title = 'Help';
				}
					foreach ([
						['name' => 'System', 'menu' => $system_menu, 'href' => null, 'id' => 'pta-menu-system'], 
						['name' => 'Interfaces', 'menu' => $interfaces_menu, 'href' => null, 'id' => 'pta-menu-interfaces'], 
						['name' => 'Firewall', 'menu' => $firewall_menu, 'href' => null, 'id' => 'pta-menu-firewall'], 
						['name' => 'Services', 'menu' => $services_menu, 'href' => null, 'id' => 'pta-menu-services'], 
						['name' => 'VPN', 'menu' => $vpn_menu, 'href' => null, 'id' => 'pta-menu-vpn'], 
						['name' => 'Status', 'menu' => $status_menu, 'href' => null, 'id' => 'pta-menu-status'], 
						['name' => 'Diagnostics', 'menu' => $diagnostics_menu, 'href' => null, 'id' => 'pta-menu-diagnostics'], 
						['name' => $help_menu_title, 'menu' => $help_menu, 'href' => '_blank', 'id' => 'pta-menu-help']] as $item):
						if ($item['name'] == 'Help' && $g['disablehelpmenu']) {
							continue;
						}

						$menu_output = output_menu($item['menu'], $item['href'], $item['name']);

						if (strlen($menu_output) > 0):
							?>
						<li class="pta-menu-item">
							<a data-toggle="collapse" data-target="#<?= $item['id'] ?>" role="button">	
							<?php if ($item['id'] =='pta-menu-system'): ?>
								<img class="icons-nav-bar" src="./System.svg" alt="">
								체계							
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-interfaces'): ?>
								<img class="icons-nav-bar" src="./Interfaces.svg" alt="">
								인터페이스
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-firewall'): ?>
								<img class="icons-nav-bar" src="./Firewall.svg" alt="">
								방화벽
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-services'): ?>
								<img class="icons-nav-bar" src="./Services.svg" alt="">
								서비스
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-vpn'): ?>
								<img class="icons-nav-bar" src="./VPN.svg" alt="">
								VPN
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-status'): ?>
								<img class="icons-nav-bar" src="./Status.svg" alt="">
								상태
								<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] =='pta-menu-diagnostics'): ?>
								<img class="icons-nav-bar" src="./Diagnostics.svg" alt="">
								진단<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php elseif ($item['id'] == 'pta-menu-help'): ?>
								<img class="icons-nav-bar" src="./Help.svg" alt="">
								돕다<span class="caret"></span>
								</a>
							<ul class="collapse item-menu-background" id="<?= $item['id'] ?>">
								<?= $menu_output ?>
							</ul>
							<?php 
						endif; ?>
						</li>
						<?php
					endif;

				endforeach ?>
			</ul>			
			</div>
      </div>
			<a class="log-out" href="/index.php?logout" usepost="">
				<i class="fa fa-sign-out" title="<?= gettext("Logout") . " (" . $_SESSION['Username'] . "@" . htmlspecialchars($system_url) . ")" ?>">&nbsp;&nbsp;&nbsp;로그 아웃</i>
			</a>

		</nav>

		<!-- PTA CONTENT -->
		<div id="pta-content" class="container <?= $menuclass ?>" <?= $columnsContainer ?>>

			<?php
			// Print a warning if current user = admin and the password hash is still set to the default value
			if ($_SESSION['Username'] == "admin") {
				$cu = getUserEntry("admin");

				if (local_backed("admin", $g['factory_shipped_password'])) {
					print('<div class="alert alert-danger">' .
						sprintf(
							gettext('%sWARNING:%s The \'admin\' account password is set to the default value. ' .
								' %s Change the password in the User Manager.%s'),
							'<strong>',
							'</strong>',
							'<a href="/system_usermanager.php?act=edit&userid=' . $cu['uid'] . '">',
							'</a>'
						) .
						'</div>');
				}
			}
			?>

			<header class="header">

				<?php
				// If you set $notitle = true BEFORE including head.inc, the page title will be supressed
				if (isset($notitle)) {
					print('<br />');
					unset($notitle);
				} else {
					if (isset($pglinks)) {
						print(genhtmltitle($pgtitle, $pglinks));
					} else {
						print(genhtmltitle($pgtitle));
					}
				}
				?>
				<ul class="context-links">

					<!-- <?php if (isset($widgets)): ?>
						<li>
							<a href="#" title="<?= gettext("Save dashboard layout") ?>" id="btnstore" class="invisible">
								<i class="fa fa-save icon-pointer"></i>
							</a>
						</li>
					<?php endif ?> -->

					<?php if ($dashboard_available_widgets_hidden): ?>
						
						<button class="button-add" onclick="$('#widget-available').toggle(360);">Add Widget</button>
						<!-- <li>
							<a onclick="$('#widget-available').toggle(360);" title="<?= gettext("Available widgets") ?>">
								<i class="fa fa-plus icon-pointer"></i>
							</a>
						</li> -->
					<?php endif ?>

					<?php if ($system_logs_filter_form_hidden): ?>
						<li>
							<a onclick="$('#filter-form').toggle(360)" title="<?= gettext("Log filter") ?>">
								<i class="fa fa-filter icon-pointer"></i>
							</a>
						</li>
					<?php endif ?>

					<?php if ($system_logs_manage_log_form_hidden):
						/* If the user does not have access to status logs settings page, then exclude the manage log panel icon from the title bar. */
						if (isAllowedPage("status_logs_settings.php")) {
							?>
							<li>
								<a onclick="$('#manage-log-form').toggle(360)" title="<?= gettext("Manage log") ?>">
									<i class="fa fa-wrench icon-pointer"></i>
								</a>
							</li>
						<?php }
					endif ?>

					<?php if ($monitoring_settings_form_hidden): ?>
						<li>
							<a onclick="$('#monitoring-settings-form').toggle(360);" title="<?= gettext("Settings") ?>">
								<i class="fa fa-wrench icon-pointer"></i>
							</a>
						</li>
					<?php endif ?>

					<?php if ($status_monitoring): ?>
						<li>
							<a class="update-graph" title="<?= gettext("Refresh Graph") ?>">
								<i class="fa fa-repeat icon-pointer"></i>
							</a>
						</li>
						<li>
							<a class="export-graph" id="export-graph" title="<?= gettext("Export Graph") ?>">
								<i class="fa fa-download icon-pointer"></i>
							</a>
						</li>
					<?php endif ?>

					<?php
					/* Determine shortcut section for XML-based packages */
					if (empty($shortcut_section) && !empty($xmlfile)) {
						$shortcut_section = basename($pagename, '.xml');
					}

					if (!$hide_service_status && !empty($shortcuts[$shortcut_section]['service']) && isAllowedPage('status_services.php')) {
						$ssvc = array();
						switch ($shortcut_section) {
							case "openvpn":
								$ssvc = find_service_by_openvpn_vpnid($vpnid);
								break;
							case "captiveportal":
								$ssvc = find_service_by_cp_zone($cpzone);
								break;
							default:
								$ssvc = find_service_by_name($shortcuts[$shortcut_section]['service']);
						}
						if (!empty($ssvc)) {
							// echo '<li>'. get_service_status_icon($ssvc, false). '</li>'; TODO: Add missing function
							echo '<li>' . get_service_control_links($ssvc, false) . '</li>';
						}
					}

					if (('' != ($link = get_shortcut_main_link($shortcut_section, false))) && (isAllowedPage($shortcuts[$shortcut_section]['main']))) {
						echo '<li>' . $link . '</li>';
					}

					if (('' != ($link = get_shortcut_status_link($shortcut_section, false))) && (isAllowedPage($shortcuts[$shortcut_section]['status']))) {
						echo '<li>' . $link . '</li>';
					}

					if (('' != ($link = get_shortcut_log_link($shortcut_section, false))) && (isAllowedPage($shortcuts[$shortcut_section]['log']))) {
						echo '<li>' . $link . '</li>';
					}

					?>
					<!-- <?php if (!$g['disablehelpicon'] && isAllowedPage("help.php")): ?>
						<li>
							<a href="<?= $helpurl ?>" target="_blank" title="<?= gettext("Help for items on this page") ?>">
								<i class="fa fa-question-circle"></i>
							</a>
						</li>
					<?php endif ?> -->
				</ul>
			</header>
			<?php
			/* if upgrade in progress, alert user */
			$warning_text = "";
			if (file_exists('/conf/needs_package_sync') && platform_booting()) {
				$warning_text = sprintf(
					gettext(
						'%1$s%3$s is booting, then packages will be reinstalled in the ' .
						'background.%2$s%1$sDo not make changes in the GUI until this is ' .
						'complete.%2$s'
					),
					'<p>',
					'</p>', $g['product_label']
				);
			} elseif (is_subsystem_dirty('packagelock')) {
				$pgtitle = array(gettext("System"), gettext("Package Manager"));
				$warning_text = sprintf(gettext('%1$sPackages are currently being ' .
					'reinstalled in the background.%2$s%1$sDo not make changes in ' .
					'the GUI until this is complete.%2$s'), '<p>', '</p>');
				$warning_text .= sprintf(
					gettext('%1$sIf the above message is still ' .
						'displayed after a couple of hours, use the \'Clear Package ' .
						'Lock\' button on the %3$s page and reinstall packages manually.' .
						'%2$s'),
					'<p>',
					'</p>',
					sprintf(
						'<a href="diag_backup.php" ' .
						'title="%1$s &gt; %2$s">%1$s &gt; %2$s</a>',
						gettext('Diagnostics'),
						htmlspecialchars(gettext('Backup & Restore'))
					)
				);
			}

			if (!empty($warning_text)) {
				print_info_box($warning_text);
			}

			/*	If this page is being remotely managed then do not allow the loading of the contents. */
			if ($config['remote_managed_pages']['item']) {
				foreach ($config['remote_managed_pages']['item'] as $rmp) {
					if ($rmp == $_SERVER['SCRIPT_NAME']) {
						print_info_box(gettext("This page is currently being managed by a remote machine."));
						include("foot.inc");
						exit;
					}
				}
			}

			// Modal notices window
			// The notices modal needs to be outside of the page display div or things get messy
			if ($display_notices):
				?>

				<div id="notices" class="modal fade" role="dialog">
					<div class="modal-dialog">
						<div class="modal-content">
							<div class="modal-header">
								<button type="button" class="close" data-dismiss="modal" aria-label="Close">
									<span aria-hidden="true">&times;</span>
								</button>

								<h3 class="modal-title" id="myModalLabel">
									<?= gettext("Notices") ?>
								</h3>
							</div>

							<div class="modal-body">
								<?php
								$noticeCategories = array();

								if (is_array($notices)) {
									foreach ($notices as $time => $notice) {
										if (!isset($noticeCategories[$notice['category']])) {
											$noticeCategories[$notice['category']] = array();
										}

										$notice['time'] = $time;
										array_push($noticeCategories[$notice['category']], $notice);
									}
								}

								foreach ($noticeCategories as $category => $catNotices): ?>
									<h4>
										<?= $category ?>
									</h4>
									<ul>
										<?php foreach ($catNotices as $notice):
											?>
											<li>
												<b>
													<?php if (!empty($notice['url'])): ?>
														<a href="<?= htmlspecialchars($notice['url']) ?>"><?= htmlspecialchars($notice['id']) ?></a> -
													<?php endif; ?>
												</b>
												<?= str_replace("\n", "<br/>", htmlspecialchars(htmlspecialchars_decode(htmlspecialchars_decode($notice['notice'])))) ?>
												<i>@
													<?= date('Y-m-d H:i:s', $notice['time']) ?>
												</i>
											</li>
										<?php endforeach; ?>
									</ul>
								<?php endforeach; ?>
							</div>

							<div class="modal-footer">
								<button type="button" class="btn btn-info" data-dismiss="modal"><i
										class="fa fa-times icon-embed-btn"></i>
									<?= gettext("Close") ?>
								</button>
								<?php if ($allow_clear_notices && isAllowedPage("/index.php")): ?>
									<button type="button" id="clearallnotices" class="btn btn-primary"><i
											class="fa fa-trash-o icon-embed-btn"></i>
										<?= gettext("Mark All as Read") ?>
									</button>
								<?php endif; ?>
							</div>
						</div>
					</div>
				</div>

				<script type="text/javascript">
					//<![CDATA[
					events.push(function () {
						$('#clearallnotices').click(function () {
							ajaxRequest = $.ajax({
								url: "/index.php",
								type: "post",
								data: {
									closenotice: "all"
								},
								success: function () {
									window.location = window.location.href;
								},
								failure: function () {
									alert("Error clearing notices!");
								}
							});
						});
					});
																																																										//]]>
				</script>

				<?php
			endif; // ($display_notices)
			
			// Get the flash Messages
			get_flash_message();