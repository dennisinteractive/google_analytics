<?php
// $Id$
/**
 * @file
 * Administrative page callbacks for the googleanalytics module.
 */

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function googleanalytics_admin_settings_form(&$form_state) {
  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('Analytics Account Settings'),
    '#collapsible' => FALSE,
  );

  $form['account']['googleanalytics_account'] = array(
    '#type' => 'textfield',
    '#title' => t('Google Analytics account number'),
    '#default_value' => variable_get('googleanalytics_account', 'UA-'),
    '#size' => 15,
    '#maxlength' => 20,
    '#required' => TRUE,
    '#description' => t('The user account number (UA-xxxxxx-x) is unique to the websites domain. You can obtain an user account from the <a href="@url">Google Analytics</a> website.', array('@url' => 'http://www.google.com/analytics/'))
  );

  $form['account']['googleanalytics_legacy_version'] = array(
    '#type' => 'checkbox',
    '#title' => t('Legacy Google Analytics'),
    '#default_value' => variable_get('googleanalytics_legacy_version', FALSE),
    '#description' => t('This will enable Legacy Google Analytics instead of most recent.<br /> Please note that Legacy Google Analytics will not receive feature updates and is not compatible with new features.'),
  );

  // Render the role overview.
  $form['roles'] = array(
    '#type' => 'fieldset',
    '#title' => t('User Role Tracking'),
    '#collapsible' => TRUE,
    '#description' => t('Define what user roles should be tracked by Google Analytics.'),
  );

  $form['roles']['googleanalytics_track__user1'] = array(
    '#type' => 'checkbox',
    '#title' => t('Admin (user 1)'),
    '#default_value' => variable_get('googleanalytics_track__user1', FALSE)
  );

  $roles = user_roles();
  foreach ($roles as $rid => $rname) {
    $form['roles']['googleanalytics_track_'. $rid] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($rname),
      '#default_value' => variable_get('googleanalytics_track_'. $rid, FALSE),
    );
  }

  $form['segmentation'] = array(
    '#type' => 'fieldset',
    '#title' => t('User Segmentation'),
    '#collapsible' => TRUE,
    '#description' => t('If your users have profile fields completed, you can track your logged in users based on a defined profile field.')
  );

  if (!module_exists('profile')) {
    $form['segmentation']['profile'] = array(
      '#type' => 'markup',
      '#value' => t('You need to activate the !profile to use this feature.', array('!profile' => l(t('Profile module'), 'admin/build/modules'))),
      '#prefix' => '<p>',
      '#suffix' => '</p>'
      );
  }
  else {
    // Compile a list of fields to show.
    $fields = array(
      'uid' => t('User ID'),
      'name' => t('Username'),
      'roles' => t('User Roles')
    );
    $result = db_query('SELECT name, title, type, weight FROM {profile_fields} ORDER BY weight');
    while ($record = db_fetch_object($result)) {
      $fields[$record->name] = $record->title;
    }

    $form['segmentation']['googleanalytics_segmentation'] = array(
      '#type' => 'select',
      '#title' => t('Track'),
      '#description' => t('Selecting one or more values allows you to track users by profile values rather than simply an IP address. To select multiple items, hold down CTRL whilst selecting fields.'),
      '#default_value' => variable_get('googleanalytics_segmentation', ''),
      '#options' => $fields,
      '#size' => 10,
      '#multiple' => TRUE
    );
  }

  // Link specific configurations.
  $form['linktracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Link tracking'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['linktracking']['googleanalytics_trackfiles'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track files'),
    '#default_value' => variable_get('googleanalytics_trackfiles', TRUE),
    '#description' => t('Enables tracking of files based on below file extensions list.')
  );
  $form['linktracking']['googleanalytics_trackfiles_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('File Extensions To Track'),
    '#default_value' => variable_get('googleanalytics_trackfiles_extensions', GA_TRACKFILES_EXTENSIONS),
    '#description' => t('A pipe separated list of file extensions that should be tracked when clicked. Example: !extensions', array('!extensions' => GA_TRACKFILES_EXTENSIONS))
  );

  // Page specific visibility configurations.
  $form['page_vis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page specific tracking settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $access = user_access('use PHP for block visibility');
  $visibility = variable_get('googleanalytics_visibility', 0);
  $pages = variable_get('googleanalytics_pages', '');

  if ($visibility == 2 && !$access) {
    $form['page_vis_settings'] = array();
    $form['page_vis_settings']['visibility'] = array('#type' => 'value', '#value' => 2);
    $form['page_vis_settings']['pages'] = array('#type' => 'value', '#value' => $pages);
  }
  else {
    $options = array(t('Add to every page except the listed pages.'), t('Add to only the listed pages.'));
    $description = t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));

    if ($access) {
      $options[] = t('Add if the following PHP code returns <code>TRUE</code> (PHP-mode, experts only).');
      $description .= ' '. t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Drupal site.', array('%php' => '<?php ?>'));
    }
    $form['page_vis_settings']['googleanalytics_visibility'] = array(
      '#type' => 'radios',
      '#title' => t('Add tracking to specific pages'),
      '#options' => $options,
      '#default_value' => $visibility,
    );
    $form['page_vis_settings']['googleanalytics_pages'] = array(
      '#type' => 'textarea',
      '#title' => t('Pages'),
      '#default_value' => $pages,
      '#description' => $description,
    );
  }

  // Advanced feature configurations.
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('You can add custom Google Analytics code here.')
  );

  $form['advanced']['googleanalytics_cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Cache tracking code file locally'),
    '#description' => t("If checked, the tracking code file is received from Google Analytics and cached locally. It is updated daily from Google's servers to ensure updates to tracking code are reflected in the local copy. Do not activate this until after Google Analytics has confirmed your tracker."),
    '#default_value' => variable_get('googleanalytics_cache', 0),
  );
  if (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PRIVATE) {
    $form['advanced']['googleanalytics_cache']['#disabled'] = TRUE;
    $form['advanced']['googleanalytics_cache']['#description'] .= ' '. t('<a href="!url">Public file transfers</a> must be enabled to allow local caching.', array('!url' => url('admin/settings/file-system', array('query' => drupal_get_destination()))));
  }

  $site_search_dependencies = '<div class="admin-dependencies">';
  $site_search_dependencies .= t('Depends on: !dependencies', array('!dependencies' => (module_exists('search') ? 'Search'. t(' (<span class="admin-enabled">enabled</span>)') : 'Search'. t(' (<span class="admin-disabled">disabled</span>)'))));
  $site_search_dependencies .= '</div>';

  // Google already have many translations, if not - they display a note to change the language.
  global $language;
  $form['advanced']['googleanalytics_site_search'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do Track Internal Search'),
    '#description' => t('If checked, internal search keywords are tracked. You must configure your Google account to use the internal query parameter <em>search</em>. For more information see <a href="!url">How do I set up Site Search for my profile</a>.', array('!url' => 'http://www.google.com/support/analytics/bin/answer.py?hl='. $language->language .'&amp;answer=75817')) . $site_search_dependencies,
    '#default_value' => variable_get('googleanalytics_site_search', FALSE),
    '#disabled' => (module_exists('search') ? FALSE : TRUE),
  );

  $form['advanced']['googleanalytics_codesnippet'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom JavaScript Code'),
    '#default_value' => variable_get('googleanalytics_codesnippet', ''),
    '#rows' => 5,
    '#description' => t('You can add custom Google Analytics <a href="!snippets">code snippets</a> here. These will be added to every page that Google Analytics appears on. <strong>Do not include the &lt;script&gt; tags</strong>, and always end your code with a semicolon (;).', array('!snippets' => 'http://drupal.org/node/248699'))
  );

  $form['advanced']['googleanalytics_js_scope'] = array(
    '#type' => 'select',
    '#title' => t('JavaScript scope'),
    '#description' => t("<strong>Warning:</strong> Google recommends adding the external JavaScript files to footer for performance reasons."),
    '#options' => array(
      'footer' => t('Footer'),
      'header' => t('Header'),
    ),
    '#default_value' => variable_get('googleanalytics_js_scope', 'footer'),
  );

  return system_settings_form($form);
}

function googleanalytics_admin_settings_form_validate($form, &$form_state) {
  if (!preg_match('/^UA-\d{4,}-\d+$/', $form_state['values']['googleanalytics_account'])) {
    form_set_error('googleanalytics_account', t('A valid Google Analytics account number is case sensitive and formated like UA-xxxxxx-x.'));
  }

  // Trim pages variable.
  $form_state['values']['googleanalytics_pages'] = trim($form_state['values']['googleanalytics_pages']);
}