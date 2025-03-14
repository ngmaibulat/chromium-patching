# Copyright 2019 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from urllib.parse import unquote

from selenium.webdriver.common.by import By
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

from test_util import create_chrome_webdriver

driver = create_chrome_webdriver()
driver.get('http://www.google.com/xhtml')

# wait for page to be loaded
wait = WebDriverWait(driver, 10)
wait.until(EC.visibility_of_element_located((By.NAME, 'q')))

search_box = driver.find_element(By.NAME, 'q')
search_box.send_keys('searchTerm')
search_box.submit()

print(unquote(driver.current_url))
driver.quit()