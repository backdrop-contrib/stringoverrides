<?php
// $Id$

/**
 * @file
 * Admin page callbacks for the String Overrides module.
 */

/**
 * Menu callback for the String Overrides module to display its administration
 */
function stringoverrides_admin($form_state = NULL, $lang = NULL) {
  // See which language we're modifying
  if (empty($lang)) {
    global $language;
    $lang = $language->language;
  }
  $form = array(
    '#cache' => TRUE,
  );
  $form['lang'] = array(
    '#type' => 'hidden',
    '#value' => $lang,
  );
  $form['string'] = array(
    '#theme' => 'stringoverrides_strings',
  );
  // Retrieve the words to display
  $delta = 0;
  if ($form_state['storage']['words']) {
    $words = $form_state['storage']['words'];
  }
  else {
    $words = variable_get('locale_custom_strings_'. $lang, array());
  }
  uksort($words, 'strcasecmp'); // Case insensitive sort by key
  foreach($words as $original => $replacement){
    $form['string'][$delta] = stringoverrides_textbox_combo($delta, TRUE, $original, $replacement);
    $delta++;
  }
  for ($index = 0; $index < 3; $index++) {
    $form['string'][$delta] = stringoverrides_textbox_combo($delta);
    $delta++;
  }
  
  $form['more_strings'] = array(
    '#type' => 'button',
    '#value' => t('More strings'),
    '#description' => t("If the amount of boxes above isn't enough, click here to add more choices."),
    '#weight' => 2,
    '#ahah' => array(
      'path' => 'admin/settings/stringoverrides/js',
      'wrapper' => 'stringoverrides-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
      'progress' => 'none',
    ),
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 3,
  );
  return $form;
} // function stringoverrides_admin

/**
 * Triggered when the user submits the administration page
 */ 
function stringoverrides_admin_submit($form, &$form_state) {
  // Format the words correctly so that they're put into the database correctly
  $words = array();
  foreach ($form_state['values']['string'] as $index => $value) {
    if (!empty($value['original']) && $value['enabled']) {
      $words[$value['original']] = $value['replacement'];
    }
  }
  
  // Save into the correct language definition
  global $language;
  $lang = $form['lang']['#value'];
  if (empty($lang)) {
    $lang = $language->language;
  }
  variable_set('locale_custom_strings_'. $lang, $words);
  
  // Output a message to the user
  drupal_set_message('Your changes have been saved.');
} // function stringoverrides_admin_submit()

/**
 * Function to return a textbox combo form
 */ 
function stringoverrides_textbox_combo($delta = 0, $enabled = TRUE, $original = '', $replacement = '') {
  $form = array(
    '#tree' => TRUE,
  );
  $form['enabled'] = array(
    '#type' => 'checkbox', 
    '#default_value' => $enabled,
    '#parents' => array('string', $delta, 'enabled'),
  );
  $form['original'] = array(
    '#type' => 'textfield',
    '#default_value' => $original,
    '#size' => 30,
    '#parents' => array('string', $delta, 'original'),
  );
  $form['replacement'] = array(
    '#type' => 'textfield',
    '#default_value' => $replacement,
    '#size' => 30,
    '#parents' => array('string', $delta, 'replacement'),
  );
  return $form;
} // function stringoverrides_textbox_combo()

/**
 * Theme the enabled box and the two text box strings
 */
function theme_stringoverrides_strings($form) {
  $headers = array(
    theme('table_select_header_cell'),
    t('Original'),
    t('Replacement'),
  );
  $rows = array();
  foreach (element_children($form) as $key) {
    // Build the table row.
    $rows[$key] = array(
      'data' => array(
        array('data' => drupal_render($form[$key]['enabled']), 'class' => 'stringoverrides-enabled'),
        array('data' => drupal_render($form[$key]['original']), 'class' => 'stringoverrides-original'),
        array('data' => drupal_render($form[$key]['replacement']), 'class' => 'stringoverrides-replacement'),
      ),
    );
    // Add any attributes on the element to the row, such as the ahah class.
    if (array_key_exists('#attributes', $form[$key])) {
      $rows[$key] = array_merge($rows[$key], $form[$key]['#attributes']);
    }
  }

  $output  = '';
  $output .= '<div id="stringoverrides-wrapper">';
  $output .= theme('table', $headers, $rows);
  $output .= '</div>';
  $output .= drupal_render($form);
  return $output;
} // function theme_stringoverrides_strings()

/**
 * Menu callback for the String Overrides module to display a new string override
 */
function stringoverrides_js() {
  $delta = count($_POST['string']);

  // Build our new form element.
  $form_element = stringoverrides_textbox_combo($delta);
  drupal_alter('form', $form_element, array(), 'stringoverrides_js');

  // Build the new form.
  $form_state = array('submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  // Add the new element to the stored form. Without adding the element to the
  // form, Drupal is not aware of this new elements existence and will not
  // process it. We retreive the cached form, add the element, and resave.
  $form = form_get_cache($form_build_id, $form_state);
  $form['string'][$delta] = $form_element;
  form_set_cache($form_build_id, $form, $form_state);
  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
  );

  // Rebuild the form.
  $form = form_builder('stringoverrides_admin', $form, $form_state);

  // Render the new output.
  $string_form = $form['string'];
  $string_form[$delta]['enabled']['#value'] = TRUE;
  $string_form[$delta]['#attributes']['class'] = empty($string_form[$delta]['#attributes']['class']) ? 'ahah-new-content' : $string_form[$delta]['#attributes']['class'] .' ahah-new-content';
  $output = theme('status_messages') . drupal_render($string_form);

  drupal_json(array('status' => TRUE, 'data' => $output));
} // function stringoverrides_js()