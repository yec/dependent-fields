<?php
// $Id: callback.inc,v 1.1.2.3 2011/01/28 11:14:35 smoothify Exp $

/**
 * The AHAH callback.
 */
function dependent_fields_callback($type_name, $field_name, $delta) {
  module_load_include('inc', 'dependent_fields', 'includes/form');
  $field = content_fields($field_name, $type_name);
  $type = (array)content_types($type_name);

  $form_state = array('storage' => NULL, 'submitted' => FALSE);

  $form_build_id = $_POST['form_build_id'];

  $form = form_get_cache($form_build_id, $form_state);

  if (!$form) {
    // Invalid form_build_id.
    drupal_set_message(t('An unrecoverable error occurred. This form was missing from the server cache. Try reloading the page and submitting again.'), 'error');
    print drupal_to_js(array('data' => theme('status_messages')));
    exit;
  }

  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;

  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form_state['ahah_controller'] = $field_name;
  $form_state['ahah_controlled'] = array();

  $form['#programmed'] = $form['#redirect'] = FALSE;

  $form_state['values'] = array();

  $form = form_builder($form_id, $form, $form_state);

  $form_element = dependent_fields_get_ahah_wrapper_element($form, $field, $type);

  // Since the after build functions don't seem to run in some cck fields form, lets
  // run it on our form element.

  dependent_fields_callback_after_build($form_element, $form_state);

  unset($form_element['#prefix'], $form_element['#suffix']);

  if (isset($form_element['_weight'])) {
    unset($form_element['_weight']);
  }

  // Render our form element
  $output = drupal_render($form_element);

  // TODO: Check we actually have multi level elements, so we only apply when required.
  // When we are using multi level dependencies, where a controlled field is also
  // a controller, the AHAH settings get lost so we need to reapply

  // Loop through the JS settings and find the settings needed for our fields.
  $javascript = drupal_add_js(NULL, NULL);
  $dependent_field_ahah_settings = array();
  if (isset($javascript['setting'])) {
    foreach ($javascript['setting'] as $settings) {
      if (isset($settings['ahah'])) {
        foreach ($settings['ahah'] as $id => $ahah_settings) {
          $url = $ahah_settings['url'];
          if (strpos($url, 'dependent_fields/callback')) {
            $dependent_field_ahah_settings[$id] = $ahah_settings;
          }
        }
      }
    }
  }

  // Add the AHAH settings needed for our new buttons.
  if (!empty($dependent_field_ahah_settings)) {
    $output .= '<script type="text/javascript">jQuery.extend(Drupal.settings.ahah, '. drupal_to_js($dependent_field_ahah_settings) .');</script>';
  }

  // Include any status messages (unlikely as we have disabled validation)
  $output = theme('status_messages') . $output;

  drupal_json(array(
    'status'   => TRUE,
    'data'     => $output,
  ));

  exit();
}


/**
 * Override Menu callback for AHAH addition of new empty widgets.
 */
function dependent_fields_add_more_js($type_name_url, $field_name) {
  module_load_include('inc', 'content', 'includes/content.node_form');
  $type = content_types($type_name_url);
  $field = content_fields($field_name, $type['type']);

  if (($field['multiple'] != 1) || empty($_POST['form_build_id'])) {
    // Invalid request.
    drupal_json(array('data' => ''));
    exit;
  }

  // Retrieve the cached form.
  $form_state = array('submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  if (!$form) {
    // Invalid form_build_id.
    drupal_json(array('data' => ''));
    exit;
  }
 
  // We don't simply return a new empty widget to append to existing ones, because
  // - ahah.js won't simply let us add a new row to a table
  // - attaching the 'draggable' behavior won't be easy
  // So we resort to rebuilding the whole table of widgets including the existing ones,
  // which makes us jump through a few hoops.

  // The form that we get from the cache is unbuilt. We need to build it so that
  // _value callbacks can be executed and $form_state['values'] populated.
  // We only want to affect $form_state['values'], not the $form itself
  // (built forms aren't supposed to enter the cache) nor the rest of $form_data,
  // so we use copies of $form and $form_data.
  $form_copy = $form;
  $form_state_copy = $form_state;
  $form_copy['#post'] = array();
  form_builder($_POST['form_id'], $form_copy, $form_state_copy);
  // Just grab the data we need.
  $form_state['values'] = $form_state_copy['values'];
  // Reset cached ids, so that they don't affect the actual form we output.
  form_clean_id(NULL, TRUE);

  // Sort the $form_state['values'] we just built *and* the incoming $_POST data
  // according to d-n-d reordering.
  unset($form_state['values'][$field_name][$field['field_name'] .'_add_more']);
  foreach ($_POST[$field_name] as $delta => $item) {
    $form_state['values'][$field_name][$delta]['_weight'] = $item['_weight'];
  }
  $form_state['values'][$field_name] = _content_sort_items($field, $form_state['values'][$field_name]);
  $_POST[$field_name] = _content_sort_items($field, $_POST[$field_name]);

  // Build our new form element for the whole field, asking for one more element.
  $form_state['item_count'] = array($field_name => count($_POST[$field_name]) + 1);
  $form_element = content_field_form($form, $form_state, $field);
  // Set the ahah path override
  $path = 'dependent_fields/js_add_more/' . $type['type'] . '/' . $field_name;
  $form_element[$field_name][$field_name . '_add_more']['#ahah']['path'] = $path;
  // Let other modules alter it.
  // We pass an empty array as hook_form_alter's usual 'form_state' parameter,
  // instead of $form_state (for reasons we may never remember).
  // However, this argument is still expected to be passed by-reference
  // (and PHP5.3 will throw an error if it isn't.) This leads to:
  $data = &$form_element;
  $empty_form_state = array();
  $data['__drupal_alter_by_ref'] = array(&$empty_form_state);
  drupal_alter('form', $data, 'content_add_more_js');

  $parent = &dependent_fields_retrieve_field_form_parent($form, $field, $type);
  
  $parent[$field_name] = $form_element[$field_name];

  // Save the new definition of the form.
  $form_state['values'] = array();
  form_set_cache($form_build_id, $form, $form_state);

  // Build the new form against the incoming $_POST values so that we can
  // render the new element.
  $delta = max(array_keys($_POST[$field_name])) + 1;
  $_POST[$field_name][$delta]['_weight'] = $delta;
  $form_state = array('submitted' => FALSE);
  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
  );
  $form = form_builder($_POST['form_id'], $form, $form_state);

  // Render the new output.
  $field_form = dependent_fields_retrieve_field_form($form, $field, $type);
  // We add a div around the new content to receive the ahah effect.
  $field_form[$delta]['#prefix'] = '<div class="ahah-new-content">'. (isset($field_form[$delta]['#prefix']) ? $field_form[$delta]['#prefix'] : '');
  $field_form[$delta]['#suffix'] = (isset($field_form[$delta]['#suffix']) ? $field_form[$delta]['#suffix'] : '') .'</div>';
  // Prevent duplicate wrapper.
  unset($field_form['#prefix'], $field_form['#suffix']);

  // If a newly inserted widget contains AHAH behaviors, they normally won't
  // work because AHAH doesn't know about those - it just attaches to the exact
  // form elements that were initially specified in the Drupal.settings object.
  // The new ones didn't exist then, so we need to update Drupal.settings
  // by ourselves in order to let AHAH know about those new form elements.
  $javascript = drupal_add_js(NULL, NULL);
  $output_js = isset($javascript['setting']) ? '<script type="text/javascript">jQuery.extend(Drupal.settings, '. drupal_to_js(call_user_func_array('array_merge_recursive', $javascript['setting'])) .');</script>' : '';

  $output = theme('status_messages') . drupal_render($field_form) . $output_js;

  // Using drupal_json() breaks filefield's file upload, because the jQuery
  // Form plugin handles file uploads in a way that is not compatible with
  // 'text/javascript' response type.
  $GLOBALS['devel_shutdown'] =  FALSE;
  print drupal_to_js(array('status' => TRUE, 'data' => $output));
  exit;
}