<?php
/*
 * @file
 * Line chart.
 * Displays a chart containing donation totals by day for a specific source.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Donations by Source: History Chart'),
  'description' => t('Displays a chart showing historical donation activity for the selected source.'),
  'category' => t('CRM Core Donations'),
  'edit form' => 'crm_core_donation_source_summary_chart_form',
  'render callback' => 'crm_core_donation_source_summary_chart',
  'admin info' => 'crm_core_donation_source_summary_chart_info',
  'defaults' => array(
    'days_to_display' => 'All',
    'height' => '',
    'xaxis_tickmarks' => '',
    'chart_color' => '',
    'line_color' => '',
    'background_color' => '',
    'include_legend' => 0,
  ),
  'all contexts' => TRUE,
);

/**
 * admin info
 */
function crm_core_donation_source_summary_chart_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    $block = new stdClass;
    $block->title = 'Donations by Source: History Chart';
    $block->content = t('Displays a chart showing historical donation activity for the selected source.');

    return $block;
  }
}

/**
 * Settings form
 */
function crm_core_donation_source_summary_chart_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['days_to_display'] = array(
    '#type' => 'textfield',
    '#title' => t('Days to display'),
    '#description' => t('Please enter the number of days to display in this chart. Use the value All to display all values for this source.'),
    '#default_value' => !empty($conf['days_to_display']) ? $conf['days_to_display'] : '90',
  );

  $form['include_legend'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display a legend'),
    '#description' => t('Check this box to include a legend in your chart.'),
    '#default_value' => !empty($conf['include_legend']) ? $conf['include_legend'] : '0',
    '#return_value' => 1,
  );

  // display settings
  $form['display_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display settings'),
    '#weight' => 5,
    '#collapsible' => FALSE,
  );

  // height
  $form['display_settings']['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#description' => t('Please enter the height of the chart, as an integer.'),
    '#default_value' => !empty($conf['height']) ? $conf['height'] : '',
  );

  // line color 
  $form['display_settings']['line_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Line color'),
    '#description' => t('Please enter the color to use for the line in valid #RRGGBB or rgba format. Leave blank to use the default color.'),
    '#default_value' => !empty($conf['line_color']) ? $conf['line_color'] : '',
  );

  // chart color 
  $form['display_settings']['chart_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Chart color'),
    '#description' => t('Please enter the color to use for the line in valid #RRGGBB or rgba format. Leave blank to use the default color.'),
    '#default_value' => !empty($conf['chart_color']) ? $conf['chart_color'] : '',
  );

  // series background color
  $form['display_settings']['background_color'] = array(
    '#type' => 'textarea',
    '#title' => t('Background color'),
    '#description' => t('Please enter the color to use for the background of the series in valid #RRGGBB or rgba format. Leave blank to use the default color. You can use gradients for this, separate each value with a new line.'),
    '#default_value' => !empty($conf['background_color']) ? $conf['background_color'] : '',
  );

  $form['display_settings']['xaxis'] = array(
    '#type' => 'fieldset',
    '#title' => t('X-axis settings'),
    '#weight' => 5,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['display_settings']['xaxis']['xaxis_tickmarks'] = array(
    '#type' => 'textfield',
    '#title' => t('Tick marks'),
    '#description' => t('Please enter the number of tickmarks to display on the xaxis.'),
    '#default_value' => !empty($conf['xaxis_tickmarks']) ? $conf['xaxis_tickmarks'] : '',
  );

  return $form;
}

/**
 * Validation handler
 */
function crm_core_donation_source_summary_chart_form_validate($form, &$form_state) {

  // timeframe
  if (!is_numeric($form_state['values']['days_to_display']) && $form_state['values']['days_to_display'] !== 'All') {
    form_set_error('days_to_display', t('The number of days must be entered as a number (i.e. 90). You can also enter \'All\' to display all records for this source.'));
  }

  // xaxis
  if (!empty($form_state['values']['xaxis_tickmarks']) && !is_numeric($form_state['values']['xaxis_tickmarks'])) {
    form_set_error('xaxis_tickmarks', t('The number of tickmarks for the xaxis must be entered as a number (i.e. 7).'));
  }

  // height
  if (!empty($form_state['values']['height']) && !is_numeric($form_state['values']['height'])) {
    form_set_error('height', t('The height of the chart must be entered as a number (i.e. 200).'));
  }
}

/**
 * Submission handler
 */
function crm_core_donation_source_summary_chart_form_submit($form, &$form_state) {

  // timeframe
  $form_state['conf']['days_to_display'] = $form_state['values']['days_to_display'];

  // legend
  $form_state['conf']['include_legend'] = $form_state['values']['include_legend'];

  // height
  $form_state['conf']['height'] = $form_state['values']['height'];

  // xaxis
  $form_state['conf']['xaxis_tickmarks'] = $form_state['values']['xaxis_tickmarks'];

  // colors
  $form_state['conf']['line_color'] = $form_state['values']['line_color'];
  $form_state['conf']['chart_color'] = $form_state['values']['chart_color'];
  $form_state['conf']['background_color'] = $form_state['values']['background_color'];
}

/**
 * Render callback
 */
function crm_core_donation_source_summary_chart($subtype, $conf, $panel_args, $context = NULL) {

  // get historical data on donations from the selected source
  // present data by day
  $sql = "SELECT SUM(fda.field_cmcd_amount_value) AS amount, 
  	UNIX_TIMESTAMP(field_cmcd_receive_date_value) AS timestamp,
  	DATE_FORMAT(field_cmcd_receive_date_value, '%b %e') AS rdate,
  	DATE_FORMAT(field_cmcd_receive_date_value, '%j') AS doy
  	FROM {field_data_field_cmcd_amount} fda 
  	JOIN {field_data_field_cmcd_receive_date} fdd
  	ON fda.entity_id = fdd.entity_id 
  	JOIN {field_data_field_cmcd_source} fds
  	ON fdd.entity_id = fds.entity_id
  	WHERE fdd.field_cmcd_receive_date_value != ''
  	  AND fdd.field_cmcd_receive_date_value IS NOT NULL";

  if ($conf['days_to_display'] !== '' && $conf['days_to_display'] !== 'ALL') {
    $sql .= " AND fdd.field_cmcd_receive_date_value > SUBDATE(NOW(), INTERVAL :days DAY) ";
  }

  $sql .= " AND fds.field_cmcd_source_value = :source
  	GROUP BY rdate, doy 
  	ORDER BY timestamp, rdate, doy DESC;";

  // run the query
  $dates = db_query($sql,
    array(
      ':source' => decode_entities($panel_args[0]),
      ':days' => $conf['days_to_display']
    )
  );

  // modify the values returned by the query. include zero values for days 
  // when donations where not collected, in order to accurately represent
  // donation activity.
  $date_item = array();
  $last_date = '';
  $last_date_ts = '';
  $series_labels = array();
  foreach ($dates as $date) {
    // check if today is greater than lastdate
    if (($date->doy - $last_date) > 1 && $last_date != '') {
      // insert some zero values for yesterday and the day after lastdate
      $date_item[] = array($last_date_ts * 1000 + 86400 * 1000, 0);
      $series_labels[] = '---';
      // add one for the day before the current record as well
      $date_item[] = array(($date->timestamp * 1000) - 86400 * 1000, 0);
      $series_labels[] = '---';
    }
    // add the date
    // $date_item[] = array($date->timestamp*1000, $date->amount);
    $date_item[] = array($date->timestamp * 1000, $date->amount);
    // record a label
    $series_labels[] = $date->rdate . '<br>' . $date->amount;
    // set lastdate
    $last_date = $date->doy;
    $last_date_ts = $date->timestamp;
  }
  $date_data = new flotData($date_item);
  $date_data->label = 'Donations by Day';
  $data[] = $date_data;

  // Configuration.
  // If setting not set for widget, we grabbing it from modules configuration.
  $height = empty($conf['height']) ? variable_get('crm_core_donation_height', '200') : $conf['height'];
  $xaxis_tickmarks = empty($conf['xaxis_tickmarks']) ? variable_get('crm_core_donation_xaxis_tickmarks', '7') : $conf['xaxis_tickmarks'];
  $chart_color = empty($conf['chart_color']) ? variable_get('crm_core_donation_chart_color', '') : $conf['chart_color'];
  $line_color = empty($conf['line_color']) ? variable_get('crm_core_donation_line_color', '') : $conf['line_color'];
  $background_color = empty($conf['background_color']) ? variable_get('crm_core_donation_background_color', '') : $conf['background_color'];
  $include_legend = empty($conf['include_legend']) ? variable_get('crm_core_donation_include_legend', 0) : $conf['include_legend'];

  // create a new line chart
  $options = new flotStyleLine();

  // add points to the chart
  $options->series->points = new stdClass();
  $options->series->points->show = TRUE;
  $options->series->points->fill = TRUE;
  $options->series->points->radius = 4;
  $options->series->points->lineWidth = 2;

  // add lines to the chart
  $options->series->lines = new stdClass();
  $options->series->lines->show = TRUE;
  $options->series->lines->fill = TRUE;
  $options->series->lines->lineWidth = 4;

  // add colors to the line
  if (!empty($line_color)) {
    $options->colors = array(
      $line_color,
    );
  }

  // add a background for the series
  if (!empty($background_color)) {
    $options->series->lines->fillColor = array(
      'colors' => explode("\n", $background_color),
    );
  }

  // add a grid
  $options->grid->borderWidth = 2;
  $options->grid->labelMargin = 8;
  if (!empty($chart_color)) {
    $options->grid->backgroundColor = $chart_color;
  }

  // add an x-axis
  $options->xaxis = new stdClass();
  $options->xaxis->mode = "time";
  $options->xaxis->ticks = (int) $xaxis_tickmarks;

  // add a y-axis
  $options->yaxis = new stdClass();
  $options->yaxis->min = 0;
  $options->series->seriesLabels = $series_labels;

  // add labels to the chart
  $options->seriesLabels = $series_labels;

  // return the chart
  $block = new stdClass();
  $block->title = t('Source History');
  $block->content = theme('flot_graph', array(
    'data' => $data,
    'options' => $options,
    'element' => array(
      'style' => 'width: 100%; height: ' . (int) $height . 'px;',
    ),
    'legend' => ($include_legend === 1) ? TRUE : FALSE,
  ));

  return $block;
}
