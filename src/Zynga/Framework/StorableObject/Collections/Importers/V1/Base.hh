<?hh // strict

namespace Zynga\Framework\StorableObject\Collections\Importers\V1;

use
  Zynga\Framework\StorableObject\Collections\V1\Interfaces\StorableCollection
;

use Zynga\Framework\Dynamic\V1\DynamicClassCreation;

use Zynga\Framework\StorableObject\V1\Interfaces\ImportInterface;
use Zynga\Framework\StorableObject\V1\Interfaces\StorableObjectInterface;

use Zynga\Framework\StorableObject\V1\Fields;

use
  Zynga\Framework\StorableObject\V1\Exceptions\ExpectedFieldCountMismatchException
;
use
  Zynga\Framework\StorableObject\V1\Exceptions\MissingKeyFromImportDataException
;
use Zynga\Framework\StorableObject\V1\Exceptions\UnsupportedTypeException;
use
  Zynga\Framework\StorableObject\V1\Exceptions\OperationNotSupportedException
;
use Zynga\Framework\StorableObject\V1\Exceptions\NoFieldsFoundException;

use Zynga\Framework\Type\V1\Interfaces\TypeInterface;

use Zynga\Framework\Exception\V1\Exception;

/**
 * Base class for collection importers. This class should be inherited
 * and each collection should have their own concrete importer.
 * Note that for importing, we expect the payload to be key value pairs
 * for the storable object to deserialize. This means that you cannot pass in
 * a vector of just values and expect the storable to deserialize correctly.
 * We want to guarantee that the values are properly assigned to each type.
 * Example:
 * // From this class type
 * class MyStorable implements StorableObjectInterface{ public StringBox $foo;}
 * //We can deserialize the payload safely with the below JSON
 * {[{"foo": "myString"}, {"foo":"myString2"}]}
 */
abstract class Base<Tv as StorableObjectInterface>
  implements ImportInterface {

  public function __construct(
    private StorableCollection<Tv> $collection,
    private classname<Tv> $rawType,
  ) {}

  public function fromVector(Vector<mixed> $data): bool {
    $this->collection->clear();

    foreach ($data as $item) {
      $storable = $this->getStorableFromItem($item);
      $this->collection->add($storable);
    }

    return true;
  }

  public function fromMap(Map<string, mixed> $data): bool {
    $this->collection->clear();

    foreach ($data as $key => $item) {
      $storable = $this->getStorableFromItem($item);
      $this->collection->add($storable);
    }

    return true;
  }

  public function fromJSON(string $payload): bool {
    $decodedJson = json_decode($payload, true);
    if (is_array($decodedJson) === false) {
      throw new UnsupportedTypeException(
        'Payload is not valid JSON. payload='.$payload,
      );
    }

    if ($this->isArrayKeyValuePair($decodedJson)) {
      $this->fromMap(new Map($decodedJson));
    } else {
      $this->fromVector(new Vector($decodedJson));
    }
    return true;
  }

  /**
   * When deserializing items for a storable object,
   * we expect each item to have a key value pair for proper
   * indexing. Otherwise, the set may not be in a known order for deserialization.
   */
  private function getStorableFromItem(mixed $item): Tv {
    $storable = DynamicClassCreation::createClassByNameGeneric(
      $this->rawType,
      Vector {},
    );

    if (is_array($item)) {
      $storable->import()->fromMap(new Map($item));
    } else if ($item instanceof Vector) {
      $storable->import()->fromVector($item);
    } else if ($item instanceof Map) {
      $storable->import()->fromMap($item);
    } else {
      throw new UnsupportedTypeException(
        'Unable to import item. item='.json_encode($item),
      );
    }
    return $storable;
  }

  private function isArrayKeyValuePair(array<mixed> $array): bool {
    return array_values($array) !== $array;
  }

  public function fromBinary(string $payload): bool {
    throw new OperationNotSupportedException(
      'method='.__METHOD__.' not supported',
    );
  }

}
